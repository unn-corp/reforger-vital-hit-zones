modded class SCR_CharacterHealthHitZone : SCR_HitZone {
	
	protected const float UNNMED_MIN_HEALTH 				= 1;
	protected const float UNNMED_OVERKILL_THRESHOLD 		= 25;
	protected const float UNNMED_SEVERE_TRAUMA_THRESHOLD	= 45;
	protected const float UNNMED_BLEEDING_THRESHOLD 		= 0.35;
	
	protected const float UNNMED_MIN_RECOVERY_PERCENT 		= 3.0;  	// Minimum recovery (% of max health)
	protected const float UNNMED_MAX_RECOVERY_PERCENT 		= 18.0; 	// Maximum recovery (% of max health)
	protected const float UNNMED_RECOVERY_CURVE 			= 4.0;		// Higher value = more punishing curve
	
	protected const float UNNMED_MIN_RESILIENCE_DRAIN_TIME 	 = 10;  	// Minimum unconscious time in seconds
	protected const float UNNMED_MAX_RESILIENCE_DRAIN_TIME 	 = 60; 	// Maximum unconscious time in seconds
	protected const float UNNMED_UNCONSCIOUS_CURVE 			 = 1.5;      // Curve steepness for unconscious time
	
	protected SCR_CharacterDamageManagerComponent m_pUnnMed_DamageManager;
	
	//-----------------------------------------------------------------------------------------------------------
	//! Initialize variables on init
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);
		m_pUnnMed_DamageManager = SCR_CharacterDamageManagerComponent.Cast(pManagerComponent);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Calculates the amount of damage the health hit zone will receive
	//! Change damage calculation while second chance is enabled
	//! Executed on all server and all clients
	override float ComputeEffectiveDamage(notnull BaseDamageContext damageContext, bool isDOT)
	{
		return UnnMed_ComputeEffectiveDamage(damageContext, isDOT);
	}
	

	//-----------------------------------------------------------------------------------------------------------
	//! Calculates the amount of damage the health hit zone will receive
	//! Change damage calculation while random survive is enabled
	//! Executed on all server and all clients
	float UnnMed_ComputeEffectiveDamage(notnull BaseDamageContext damageContext, bool isDOT)
	{
	    float effectiveDamage = super.ComputeEffectiveDamage(damageContext, isDOT);
		
		// Hitzone not valid for cheating death, return standard damage
		if (!m_pUnnMed_DamageManager.UnnMed_HasCheatingDeathOnHitZone(damageContext.struckHitZone))
			return effectiveDamage;
		
	    float health = GetHealth();
	    float healthAfterDamage = health - effectiveDamage;
		
		// If they survive normally, or have cheated death already, return standard damage
	    if (healthAfterDamage >= UNNMED_MIN_HEALTH  || m_pUnnMed_DamageManager.UnnMed_WasCheatingDeathTrigged())
		{
		    return effectiveDamage;
		}
		
		
	    SCR_CharacterHitZone hitZone = SCR_CharacterHitZone.Cast(damageContext.struckHitZone);
	    
		#ifdef WORKBENCH
		Print("-");
		Print("health:            " +  health.ToString());
		Print("effectiveDamage:   " +  effectiveDamage.ToString());
		Print("damageType:        " +  typename.EnumToString(EDamageType, damageContext.damageType));
		Print("healthAfterDamage: " +  healthAfterDamage.ToString());
	    Print(" ");
		if(hitZone)
			Print("HitZone:           " + damageContext.struckHitZone.GetName());
		else
			Print("invalid HitZone, ignoring: " + damageContext.struckHitZone.GetName());
	    Print(" ");
		#endif 
		
	    if (!hitZone)
			return effectiveDamage;
		
		int hitZoneGroup = hitZone.GetHitZoneGroup();
		bool isCentreMass = hitZoneGroup == ECharacterHitZoneGroup.LOWERTORSO || 
			 				hitZoneGroup == ECharacterHitZoneGroup.UPPERTORSO ||
			 				hitZoneGroup == ECharacterHitZoneGroup.HEAD;
			
	    // If damage is too large, and hits a vital zone, character dies regardless
	    if (effectiveDamage > UNNMED_SEVERE_TRAUMA_THRESHOLD && isCentreMass) 
		{
			#ifdef WORKBENCH
			Print("Killed by Severe Trauma to " + hitZone.GetName());
	    	Print(" ");
			#endif
	        return effectiveDamage;
		}
	    
	    // Calculate overkill damage
	    float overkillDamage = Math.AbsFloat(healthAfterDamage);
	    float normalizedOverkill = overkillDamage / UNNMED_OVERKILL_THRESHOLD;
		
		#ifdef WORKBENCH
		Print("overkillDamage:    " +  overkillDamage.ToString());
		Print("normalizedOverkill:" +  normalizedOverkill.ToString());
		#endif
	    
	    // If overkill exceeds threshold, character dies
	    if (overkillDamage >= UNNMED_OVERKILL_THRESHOLD) 
		{
			#ifdef WORKBENCH
			Print("Killed by overkill damage");
	    	Print(" ");
			#endif
			return effectiveDamage;
		}
	
		array<HitZone> bleedingHitZones = m_pUnnMed_DamageManager.GetBleedingHitZones();
	
        if (normalizedOverkill >= UNNMED_BLEEDING_THRESHOLD && 
			(bleedingHitZones == null ||  !bleedingHitZones.Contains(hitZone))
		   ) 
		{
			#ifdef WORKBENCH
			Print("Adding bleed to "+ hitZone.GetName());
    		Print(" ");
			#endif
            m_pUnnMed_DamageManager.AddBleedingEffectOnHitZone(hitZone, damageContext.colliderID);
		}
		
		if (isCentreMass) {
			float resilienceScale = Math.Pow(normalizedOverkill, UNNMED_UNCONSCIOUS_CURVE);
			float resilienceTime = UNNMED_MIN_RESILIENCE_DRAIN_TIME + (resilienceScale * (UNNMED_MAX_RESILIENCE_DRAIN_TIME - UNNMED_MIN_RESILIENCE_DRAIN_TIME));
			#ifdef WORKBENCH
			Print("Adding resilience drain for " + resilienceTime.ToString());
    		Print(" ");
			#endif
			m_pUnnMed_DamageManager.UnnMed_DealResilienceDamage(100, resilienceTime);
		}
        
        m_pUnnMed_DamageManager.UnnMed_SetCheatingDeathTrigged(true);
	    
	    // Calculate recovery based on a power curve
	    float recoveryScale = Math.Pow(1.0 - normalizedOverkill, UNNMED_RECOVERY_CURVE);
	    
	    // Scale between min and max recovery percentages
	    float recoveryPercent = UNNMED_MIN_RECOVERY_PERCENT + (recoveryScale * (UNNMED_MAX_RECOVERY_PERCENT - UNNMED_MIN_RECOVERY_PERCENT));
	    
	    // Calculate health to recover (as percentage of max health)
	    float maxHealth = GetMaxHealth();
	    float healthToRecover = maxHealth * (recoveryPercent / 100.0);
	    
		#ifdef WORKBENCH
		Print("recoveryScale:     " +  recoveryScale.ToString());
		Print("recoveryPercent:   " +  recoveryPercent.ToString());
	    Print(" ");
		Print("Incapacitated with " + (health + healthToRecover).ToString() + " health remaining");
	    Print(" ");
		#endif
		
	    // Return damage that would leave the character with the calculated health
	    return health - healthToRecover;
	}
}