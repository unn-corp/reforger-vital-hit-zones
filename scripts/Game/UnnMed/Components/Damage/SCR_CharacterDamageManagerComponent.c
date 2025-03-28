
modded class SCR_CharacterDamageManagerComponent {
	
	// We only notify the replication system about changes of these members on initialization
	// After init, each proxy is itself responsible for updating these members
	// Having them as RplProp also ensures that JIPs receive the current state from the server
	[RplProp()]
	protected bool m_bUnnMed_Initialized = false;
	[RplProp()]
	protected bool m_bUnnMed_HasCheatingDeath = true;
	[RplProp()]
	protected bool m_bUnnMed_CheatingDeathTriggered = false;
	
	//-----------------------------------------------------------------------------------------------------------
	//! Initialize Unn medical on a character damage manager (Called on the server)
	void UnnMed_Initialize()
	{
		if (m_bUnnMed_Initialized)
			return;
				
		UnnMed_EnableCheatingDeath(true);
		// Damage calculations are done on all machines, so we have to broadcast the init
		m_bUnnMed_Initialized = true;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if Unn Medical has been initialized
	bool UnnMed_IsInitialized()
	{
		return m_bUnnMed_Initialized;
	}

	//------------------------------------------------------------------------------------------------
	//! Enable/disable random survive
	void UnnMed_EnableCheatingDeath(bool enable)
	{
		m_bUnnMed_HasCheatingDeath = enable;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if random survive is enabled
	bool UnnMed_HasCheatingDeath()
	{
		return m_bUnnMed_HasCheatingDeath;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns true if random survive is enabled for the given hit zone
	bool UnnMed_HasCheatingDeathOnHitZone(HitZone hitZone)
	{
		if (!m_bUnnMed_HasCheatingDeath)
			return false;
		
		return (hitZone != m_pHeadHitZone);
	}
	
	//------------------------------------------------------------------------------------------------
	//! To be set true when random survive was used
	void UnnMed_SetCheatingDeathTrigged(bool isTriggered)
	{
		m_bUnnMed_CheatingDeathTriggered = isTriggered;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if random survive was used
	bool UnnMed_WasCheatingDeathTrigged()
	{
		return m_bUnnMed_CheatingDeathTriggered;
	}
	
	void UnnMed_DealResilienceDamage(float amount, float duration) 
	{
		SCR_CharacterResilienceHitZone hitZone = GetResilienceHitZone();
		
		// This code is handled on authority only
		if (hitZone.IsProxy())
			return;
		
		#ifdef WORKBENCH
		Print("res: " + hitZone.GetHealthScaled().ToString());
		#endif
		
		UnnMed_SlappingDamageEffect hitZoneSlapping = new UnnMed_SlappingDamageEffect();
		hitZoneSlapping.SetDPS(amount);
		hitZoneSlapping.SetMaxDuration(duration);
		hitZoneSlapping.SetDamageType(EDamageType.HEALING);
		hitZoneSlapping.SetAffectedHitZone(hitZone);
		hitZoneSlapping.SetInstigator(GetInstigator());
		AddDamageEffect(hitZoneSlapping);
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
