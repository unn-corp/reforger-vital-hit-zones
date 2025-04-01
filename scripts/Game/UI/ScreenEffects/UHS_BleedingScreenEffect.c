modded class SCR_BleedingScreenEffect 
{
	protected bool m_VHS_IsPlayerDoomed;
	
	override void OnDamageEffectAdded(notnull SCR_DamageEffect dmgEffect)
	{
		super.OnDamageEffectAdded(dmgEffect);
		
		if(m_DamageManager.VHS_IsPlayerDoomed() && !m_VHS_IsPlayerDoomed) 
		{
			m_VHS_IsPlayerDoomed = true;
			SCR_UISoundEntity.SetSignalValueStr("VHS_Doomed", 1);
		}
	}
	
	override void OnDamageEffectRemoved(notnull SCR_DamageEffect dmgEffect)
	{
		m_VHS_IsPlayerDoomed = m_DamageManager.VHS_IsPlayerDoomed();
	}
}