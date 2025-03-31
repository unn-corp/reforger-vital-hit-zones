class VHS_VitalHitZone : SCR_CharacterHitZone {
	
	// Define the vital hitzone as an offset from the bone/collider instead of a fixed position
	[Attribute(desc: "HitZone Offset from Bone", defvalue: "0 0.1 0.05")]
	protected vector m_vHitZoneOffset;
	
	[Attribute(desc: "HitZone Radius", defvalue: "0.05")]
	protected float m_fHitZoneSize;
	
	protected SCR_CharacterDamageManagerComponent m_pVHS_DamageManager;
	protected IEntity m_Owner;
	protected int m_ColliderDescriptorIndex;
	
	#ifdef WORKBENCH
	ref array<ref Shape> m_aDbgSamplePositionsShapes;
	#endif
	
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);
		m_pVHS_DamageManager = SCR_CharacterDamageManagerComponent.Cast(pManagerComponent);
		m_Owner = pOwnerEntity;
		
		array<int> colliderIDs = {};
		GetColliderIDs(colliderIDs);
		
		m_ColliderDescriptorIndex = GetColliderDescriptorIndex(colliderIDs[0]);
		
		#ifdef WORKBENCH
		m_aDbgSamplePositionsShapes = {};
		
		// Visualize current bone/collider position and the vital zone in workbench
		vector colliderTransformLocalSpace[4];
		int boneIndex;
		int boneNode;
		
		if(TryGetColliderDescription(m_Owner, m_ColliderDescriptorIndex, colliderTransformLocalSpace, boneIndex, boneNode)) 
		{
			vector vitalZonePos = CalculateVitalZonePosition(colliderTransformLocalSpace);
			Debug_DrawSphereAtPos(colliderTransformLocalSpace[3], m_aDbgSamplePositionsShapes, COLOR_BLUE, m_fHitZoneSize);
			Debug_DrawSphereAtPos(vitalZonePos, m_aDbgSamplePositionsShapes, COLOR_RED, m_fHitZoneSize);
		}
		#endif
	}
	
	// Calculate the current position of the vital zone based on bone transform
	protected vector CalculateVitalZonePosition(vector colliderTransform[4])
	{	
		vector transformedOffset = vector.Zero;
		transformedOffset[0] = vector.Dot(m_vHitZoneOffset, colliderTransform[0]);
		transformedOffset[1] = vector.Dot(m_vHitZoneOffset, colliderTransform[1]);
		transformedOffset[2] = vector.Dot(m_vHitZoneOffset, colliderTransform[2]);
		
		return colliderTransform[3] + transformedOffset;
	}
	
	override float ComputeEffectiveDamage(notnull BaseDamageContext damageContext, bool isDOT)
	{	
		float effectiveDamage = super.ComputeEffectiveDamage(damageContext, isDOT);
		if(damageContext.damageType != EDamageType.KINETIC)
		{
			return effectiveDamage;
		}
		
		vector colliderTransformLocalSpace[4];
		int boneIndex;
		int boneNode;
		
		vector hitPos = m_Owner.CoordToLocal(damageContext.hitPosition);
		vector hitDir = m_Owner.VectorToLocal(damageContext.hitDirection);
		
		if(!damageContext.struckHitZone.TryGetColliderDescription(m_Owner, m_ColliderDescriptorIndex, colliderTransformLocalSpace, boneIndex, boneNode)) 
		{
			return 0;
		}
		
		#ifdef WORKBENCH
		Print("dt:" + damageContext.damageType);
		Print("dv:" + damageContext.damageValue);
			
		Print("dir: " + hitDir.ToString());
		Print("pos: " + hitPos.ToString());
		Print("cpos 0: " + colliderTransformLocalSpace[0].ToString());
		Print("cpos 1: " + colliderTransformLocalSpace[1].ToString());
		Print("cpos 2: " + colliderTransformLocalSpace[2].ToString());
		Print("cpos 3: " + colliderTransformLocalSpace[3].ToString());
		Print(damageContext.damageValue.ToString());
		#endif
		
		// Calculate current vital zone position based on bone transform
		vector vitalZonePos = CalculateVitalZonePosition(colliderTransformLocalSpace);
		
		// Check if the bullet path intersects the vital zone
		if(!CheckBulletHitsVitalZone(hitPos, hitDir, vitalZonePos, m_fHitZoneSize)) {
			return 0;
		}
		
		#ifdef WORKBENCH
		Print("VITAL HIT!");
		Print("Damage: " + effectiveDamage.ToString());
		#endif
		
		return effectiveDamage;
	}
	
	/**
	 * Check if a bullet path intersects with the spherical vital zone
	 */
	protected bool CheckBulletHitsVitalZone(vector bulletPos, vector bulletDir, vector spherePos, float sphereRadius)
	{
		// game engine calls ComputeEffectiveDamage with zero direction when calculating if the hit should cause a bleed.
		// because we already responded once with a non zero computed damage value, we know we're getting called again with zero direction
		// so just say it is a vital hit.
	    if (bulletDir == vector.Zero)
	        return true;
	
	    // is going toward or away from?
	    float projection = vector.Dot(spherePos - bulletPos, bulletDir.Normalized());
	
	    if (projection < 0) 
	        return false;
	
	    // check ray intersection with vital hitzone sphere
	    return vector.Distance(bulletPos + (bulletDir.Normalized() * projection), spherePos) <= sphereRadius;
	}
	
	
	#ifdef WORKBENCH
	protected void Debug_DrawSphereAtPos(vector v, array<ref Shape> dbgShapes, int color = COLOR_BLUE, float size = 0.03, ShapeFlags shapeFlags = ShapeFlags.VISIBLE)
	{
		shapeFlags = ShapeFlags.NOOUTLINE | ShapeFlags.NOZBUFFER | shapeFlags;
		
		vector matx[4];
		Math3D.MatrixIdentity4(matx);
		matx[3] = v;
		Shape s = Shape.CreateSphere(color, shapeFlags, m_Owner.GetOrigin(), size);
		s.SetMatrix(matx);
		dbgShapes.Insert(s);
	}
	#endif
}