//------------------------------------------------------------------------------------------------
//! Introduce a random survive, which gets triggered when the character would have died without
//! falling unconscious.
modded class SCR_CharacterControllerComponent : CharacterControllerComponent
{
	protected SCR_CharacterDamageManagerComponent m_pUnnMed_DamageManager;
	protected const float UNNMED_CHEATING_DEATH_DEACTIVATION_TIMEOUT_MS = 1000;
	
	//------------------------------------------------------------------------------------------------
	//! Initialize member variables
	override void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		m_pUnnMed_DamageManager = SCR_CharacterDamageManagerComponent.Cast(owner.FindComponent(SCR_CharacterDamageManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add/remove random survive when life state changes
	override void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		super.OnLifeStateChanged(previousLifeState, newLifeState);
		
		// Only run if ACE Medical has been initialized for this character
		if (!m_pUnnMed_DamageManager.UnnMed_IsInitialized())
			return;
		
		// OnLifeStateChanged sometimes gets triggered without a change in state
		if (previousLifeState == newLifeState)
			return;
		
		switch (newLifeState)
		{
			// Add random survive when revived
			case ECharacterLifeState.ALIVE:
			{
				GetGame().GetCallqueue().Remove(m_pUnnMed_DamageManager.UnnMed_EnableCheatingDeath);
				m_pUnnMed_DamageManager.UnnMed_EnableCheatingDeath(true);
				m_pUnnMed_DamageManager.UnnMed_SetCheatingDeathTrigged(false);
				break;
			}
			
			// Schedule removal of random survive when falling unconscious
			case ECharacterLifeState.INCAPACITATED:
			{
				GetGame().GetCallqueue().CallLater(m_pUnnMed_DamageManager.UnnMed_EnableCheatingDeath, UNNMED_CHEATING_DEATH_DEACTIVATION_TIMEOUT_MS, false, false);
				break;
			}
			
			// Remove random survive when dead
			case ECharacterLifeState.DEAD:
			{
				GetGame().GetCallqueue().Remove(m_pUnnMed_DamageManager.UnnMed_EnableCheatingDeath);
				m_pUnnMed_DamageManager.UnnMed_EnableCheatingDeath(false);
				break;
			}
		}
	}
}
