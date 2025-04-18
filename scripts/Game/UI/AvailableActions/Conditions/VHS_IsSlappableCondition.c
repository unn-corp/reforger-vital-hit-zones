//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class VHS_IsSlappableCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Returns true if the release action is available
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{		
		if (!data)
			return false;
		
		return GetReturnResult(!data.GetIsCharacterConscious());
	}
}
