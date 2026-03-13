#pragma once

#include "Framework/Commands/Commands.h"


class FCoreManagerUICommands : public TCommands<FCoreManagerUICommands>
{
public:
	FCoreManagerUICommands() : TCommands<FCoreManagerUICommands> (TEXT("CoreManager"), FText::FromString(TEXT("Core Manager UI Commands")), NAME_None, TEXT("CoreManager")) 
	{

	}

	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> LockActorSelection;
	TSharedPtr<FUICommandInfo> UnlockActorSelection;
};