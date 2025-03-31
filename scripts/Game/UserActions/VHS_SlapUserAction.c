//Stupid but fun, slap an incapacitated person to wake them up
//------------------------------------------------------------------------------------------------
class VHS_SlapUserAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		ChimeraCharacter ownerChar = ChimeraCharacter.Cast(GetOwner());
		if (!ownerChar)
			return false;
		
		
		SCR_CharacterControllerComponent ownerCharCtrl = SCR_CharacterControllerComponent.Cast(ownerChar.GetCharacterController());
		SCR_CharacterDamageManagerComponent ownerCharDmg = SCR_CharacterDamageManagerComponent.Cast(ownerChar.GetDamageManager());
		if (!ownerCharDmg || !ownerCharCtrl || !ownerCharCtrl.IsUnconscious() || ownerCharCtrl.GetLifeState() == ECharacterLifeState.DEAD)
			return false;

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		ChimeraCharacter userChar = ChimeraCharacter.Cast(user);
		if (!userChar)
			return false;
		
		ChimeraCharacter ownerChar = ChimeraCharacter.Cast(GetOwner());
		if (!ownerChar)
			return false;
		
		SCR_CharacterControllerComponent ownerCharCtrl = SCR_CharacterControllerComponent.Cast(ownerChar.GetCharacterController());
		SCR_CharacterDamageManagerComponent ownerCharDmg = SCR_CharacterDamageManagerComponent.Cast(ownerChar.GetDamageManager());
		if(!ownerCharDmg || !ownerCharCtrl || !ownerCharCtrl.IsUnconscious() || ownerCharCtrl.GetLifeState() == ECharacterLifeState.DEAD)
			return false;

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		ChimeraCharacter ownerChar = ChimeraCharacter.Cast(pOwnerEntity);
		SCR_CharacterDamageManagerComponent.Cast(ownerChar.GetDamageManager()).VHS_DealResilienceDamage(-35, 1);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Only run PerformAction on server
	override bool CanBroadcastScript() { return false; };
}
