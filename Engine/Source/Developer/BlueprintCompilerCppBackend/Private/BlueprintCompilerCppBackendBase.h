// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma  once

#include "IBlueprintCompilerCppBackendModule.h"

/** The class generates all native code except of function body (notice InnerFunctionImplementation in not implemented) */
class FBlueprintCompilerCppBackendBase : public IBlueprintCompilerCppBackend
{
protected:
	struct FFunctionLabelInfo
	{
		TMap<FBlueprintCompiledStatement*, int32> StateMap;
		int32 StateCounter;

		FFunctionLabelInfo()
		{
			StateCounter = 0;
		}

		int32 StatementToStateIndex(FBlueprintCompiledStatement* Statement)
		{
			int32& Index = StateMap.FindOrAdd(Statement);
			if (Index == 0)
			{
				Index = ++StateCounter;
			}

			return Index;
		}
	};

	TArray<FFunctionLabelInfo> StateMapPerFunction;
	TMap<FKismetFunctionContext*, int32> FunctionIndexMap;

	FString CppClassName;

	FStringOutputDevice Header;
	FStringOutputDevice Body;
public:

	// IBlueprintCompilerCppBackend implementation
	virtual void GenerateCodeFromClass(UClass* SourceClass, TIndirectArray<FKismetFunctionContext>& Functions, bool bGenerateStubsOnly) override;
	virtual void GenerateCodeFromEnum(UUserDefinedEnum* SourceEnum) override;
	virtual void GenerateCodeFromStruct(UUserDefinedStruct* SourceStruct) override;

	virtual const FString& GetBody()	const override { return Body; }
	virtual const FString& GetHeader()	const override { return Header; }
	// end of IBlueprintCompilerCppBackend implementation

	virtual ~FBlueprintCompilerCppBackendBase() {}
private:
	void Emit(FStringOutputDevice& Target, const TCHAR* Message)
	{
		Target += Message;
	}

protected:
	/*
		ConstructFunction generates definition, declaration and local variables. The actual implementation is generated by InnerFunctionImplementation.
		For example - this code is generated by ConstructFunction:

			void AMyActor_Child_C::BoxBox_Implementation(FBox2D NewParam,  FBox2D& NewParam1)
			{
				FBox2D CallFunc_BoxBox_NewParam1{};

				// this part is generated by InnerFunctionImplementation

				return;
			}

	*/
	virtual FString InnerFunctionImplementation(FKismetFunctionContext& FunctionContext, bool bUseSwitchState) PURE_VIRTUAL(FBlueprintCompilerCppBackendBase::InnerFunctionImplementation, return FString(););

	void EmitStructProperties(FStringOutputDevice& Target, UStruct* SourceClass);

	/** Emits local variable declarations for a function */
	void DeclareLocalVariables(FKismetFunctionContext& FunctionContext, TArray<UProperty*>& LocalVariables);
	
	/** Builds both the header declaration and body implementation of a function */
	void ConstructFunction(FKismetFunctionContext& FunctionContext, bool bGenerateStubOnly);

	void EmitFileBeginning(const FString& CleanName, UStruct* SourceStruct);

	int32 StatementToStateIndex(FKismetFunctionContext& FunctionContext, FBlueprintCompiledStatement* Statement)
	{
		int32 Index = FunctionIndexMap.FindChecked(&FunctionContext);
		return StateMapPerFunction[Index].StatementToStateIndex(Statement);
	}
};
