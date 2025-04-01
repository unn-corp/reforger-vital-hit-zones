
modded class SCR_CharacterDamageManagerComponent {

	[Attribute(desc: "Define the vital hitzones")]
	protected array<ref VHS_VitalHitZone> m_aVitalHitZones;
	
	// We only notify the replication system about changes of these members on initialization
	// After init, each proxy is itself responsible for updating these members
	// Having them as RplProp also ensures that JIPs receive the current state from the server
	[RplProp()]
	protected bool m_bVHS_Initialized = false;
	[RplProp()]
	protected bool m_bVHS_HasCheatingDeath = true;
	[RplProp()]
	protected bool m_bVHS_CheatingDeathTriggered = false;
	[RplProp()]
	protected bool m_bVHS_IsPlayerDoomed = false;
	
	//-----------------------------------------------------------------------------------------------------------
	//! Initialize Unn medical on a character damage manager (Called on the server)
	void VHS_Initialize()
	{
		if (m_bVHS_Initialized)
			return;
				
		VHS_EnableCheatingDeath(true);
		// Damage calculations are done on all machines, so we have to broadcast the init
		m_bVHS_Initialized = true;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if Unn Medical has been initialized
	bool VHS_IsInitialized()
	{
		return m_bVHS_Initialized;
	}
	
	void VHS_SetPlayerDoomed(bool doomed) 
	{
		m_bVHS_IsPlayerDoomed = doomed;
	}
	
	bool VHS_IsPlayerDoomed()
	{
		return m_bVHS_IsPlayerDoomed;
	}

	//------------------------------------------------------------------------------------------------
	//! Enable/disable random survive
	void VHS_EnableCheatingDeath(bool enable)
	{
		m_bVHS_HasCheatingDeath = enable;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if random survive is enabled
	bool VHS_HasCheatingDeath()
	{
		return m_bVHS_HasCheatingDeath;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns true if random survive is enabled for the given hit zone
	bool VHS_HasCheatingDeathOnHitZone(HitZone hitZone)
	{
		if (!m_bVHS_HasCheatingDeath)
			return false;
		
		return (hitZone != m_pHeadHitZone);
	}
	
	//------------------------------------------------------------------------------------------------
	//! To be set true when random survive was used
	void VHS_SetCheatingDeathTrigged(bool isTriggered)
	{
		m_bVHS_CheatingDeathTriggered = isTriggered;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if random survive was used
	bool VHS_WasCheatingDeathTrigged()
	{
		return m_bVHS_CheatingDeathTriggered;
	}
	
	void VHS_DealResilienceDamage(float amount, float duration) 
	{
		SCR_CharacterResilienceHitZone hitZone = GetResilienceHitZone();
		
		// This code is handled on authority only
		if (hitZone.IsProxy())
			return;
		
		#ifdef VHS_Debug
		Print("res: " + hitZone.GetHealthScaled().ToString());
		#endif
		
		VHS_SlappingDamageEffect hitZoneSlapping = new VHS_SlappingDamageEffect();
		hitZoneSlapping.SetDPS(amount);
		hitZoneSlapping.SetMaxDuration(duration);
		hitZoneSlapping.SetDamageType(EDamageType.HEALING);
		hitZoneSlapping.SetAffectedHitZone(hitZone);
		hitZoneSlapping.SetInstigator(GetInstigator());
		AddDamageEffect(hitZoneSlapping);
	}

	//-----------------------------------------------------------------------------------------------------------
	/*! Add bleeding effect to hitzone
	\param hitZone Hitzone to get bleeding rate from and add effect to
	\param colliderDescriptorIndex Collider descriptor index
	*/
	override void AddBleedingEffectOnHitZone(notnull SCR_CharacterHitZone hitZone, int colliderDescriptorIndex = -1)
	{
		// This code is handled on authority only
		if (hitZone.IsProxy())
			return;
		
		// In case bleeding is started outside of normal context, full health will prevent DOT. This block will circumvent this issue
		float hitZoneDamageMultiplier = hitZone.GetHealthScaled();
		float bleedingRate = hitZone.GetMaxBleedingRate() - hitZone.GetMaxBleedingRate() * hitZoneDamageMultiplier;
		
		//ignore scenario/GM scaling multipliers for doomed bleeds
		if(!m_bVHS_IsPlayerDoomed)
			bleedingRate *= GetBleedingScale();

		SCR_BleedingDamageEffect hitZoneBleeding = new SCR_BleedingDamageEffect();
		if (colliderDescriptorIndex == -1)
			colliderDescriptorIndex = Math.RandomInt(0, hitZone.GetNumColliderDescriptors() - 1);
			
		hitZoneBleeding.m_iColliderDescriptorIndex = colliderDescriptorIndex;
		hitZoneBleeding.SetDPS(bleedingRate);
		hitZoneBleeding.SetMaxDuration(0);
		hitZoneBleeding.SetDamageType(EDamageType.BLEEDING);
		hitZoneBleeding.SetAffectedHitZone(hitZone);
		hitZoneBleeding.SetInstigator(GetInstigator());
		AddDamageEffect(hitZoneBleeding);
	}
	
	/*-----------------------------------------------------------------------------------------------------------
	//! Check whether character health state meets requirements for consciousness
	override bool ShouldBeUnconscious()
	{
		HitZone bloodHZ = GetBloodHitZone();
		if (!bloodHZ)
			return false;
		
		ECharacterBloodState bloodState = bloodHZ.GetDamageState();
		if (bloodHZ.GetDamageStateThreshold(bloodState) <= bloodHZ.GetDamageStateThreshold(ECharacterBloodState.UNCONSCIOUS))	
			return true;
				
		HitZone resilienceHZ = GetResilienceHitZone();
		if (!resilienceHZ)
			return false;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return false;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;

		ECharacterResilienceState resilienceState = resilienceHZ.GetDamageState();

		if (controller.IsUnconscious())
		{		
			if (resilienceHZ.GetDamageStateThreshold(resilienceState) <= resilienceHZ.GetDamageStateThreshold(ECharacterResilienceState.FAINTING))
				return true;
		}
		else
		{
			if (resilienceHZ.GetDamageStateThreshold(resilienceState) <= resilienceHZ.GetDamageStateThreshold(ECharacterResilienceState.UNCONSCIOUS))
				return true;
		}
			
		return false;
	}*/
}
