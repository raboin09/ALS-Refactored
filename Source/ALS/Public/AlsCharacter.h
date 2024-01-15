#pragma once

#include "GameFramework/Character.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Settings/AlsMantlingSettings.h"
#include "State/AlsLocomotionState.h"
#include "State/AlsMantlingState.h"
#include "State/AlsMovementBaseState.h"
#include "State/AlsRagdollingState.h"
#include "State/AlsRollingState.h"
#include "State/AlsViewState.h"
#include "Utility/AlsGameplayTags.h"
#include "AlsCharacter.generated.h"

struct FAlsMantlingParameters;
struct FAlsMantlingTraceSettings;
class UAlsCharacterMovementComponent;
class UAlsCharacterSettings;
class UAlsMovementSettings;
class UAlsAnimationInstance;
class UAlsMantlingSettings;

UCLASS(AutoExpandCategories = ("Settings|Als Character", "Settings|Als Character|Desired State", "CUSTOM|Als Character"))
class ALS_API AAlsCharacter : public ACharacter
{
	GENERATED_BODY()
#pragma region ActorOverrides
	//////////////////////////////////
	/// AActor Overrides
	//////////////////////////////////
public:
	explicit AAlsCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* Property) const override;
#endif
	//////////////////////////////////
	/// ~~ End AActor ~~
	//////////////////////////////////
#pragma endregion ActorOverrides

#pragma region PawnOverrides
	//////////////////////////////////
	/// APawn Overrides
	//////////////////////////////////
	virtual void PossessedBy(AController* NewController) override;
	virtual void PostRegisterAllComponents() override;
	virtual void PostInitializeComponents() override;
	FORCEINLINE virtual FRotator GetViewRotation() const override { return ViewState.Rotation; }
	virtual void FaceRotation(FRotator Rotation, float DeltaTime) override final {} // Empty to remove ability to set rotation from things outside of ALS
	//////////////////////////////////
	/// ~~ End APawn ~~
	//////////////////////////////////
#pragma endregion PawnOverrides

#pragma region CharacterOverrides
	//////////////////////////////////
	/// ACharacter Overrides
	//////////////////////////////////
	virtual void PostNetReceiveLocationAndRotation() override;
	virtual void OnRep_ReplicatedBasedMovement() override;
	virtual void Restart() override;
	// This allows the ACharacter::Crouch() function to execute properly when bIsCrouched is true.
	// TODO Wait for https://github.com/EpicGames/UnrealEngine/pull/9558 to be merged into the engine.
	FORCEINLINE virtual bool CanCrouch() const override { return bIsCrouched || Super::CanCrouch(); }
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void Jump() override;
	virtual void OnJumped_Implementation() override;
	//////////////////////////////////
	/// ~~ End ACharacter ~~
	//////////////////////////////////
#pragma endregion CharacterOverrides
	
	//////////////////////////////////
	/// Public Event Virtuals
	//////////////////////////////////
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode = 0) override;	
	
	//////////////////////////////////
	/// Public Functions
	//////////////////////////////////
	void CorrectViewNetworkSmoothing(const FRotator& NewTargetRotation, bool bRelativeTargetRotation);
	void CharacterMovement_OnPhysicsRotation(float DeltaTime);
	bool IsRollingAllowedToStart(const UAnimMontage* Montage) const;
	bool IsRagdollingAllowedToStart() const;
	bool IsRagdollingAllowedToStop() const;
	void FinalizeRagdolling();
	void SetLocomotionAction(const FGameplayTag& NewLocomotionAction);
	
	//////////////////////////////////
	/// Public K2 Native Events
	//////////////////////////////////
	UFUNCTION(BlueprintNativeEvent, Category = "Als Character")
	bool IsMantlingAllowedToStart() const;
	UFUNCTION(BlueprintNativeEvent, Category = "Als Character")
	UAnimMontage* SelectRollMontage();

	//////////////////////////////////
	/// Public K2 Callable Events
	//////////////////////////////////
	UFUNCTION(BlueprintCallable, Category = "ALS|Character")
	void StartRolling(float PlayRate = 1.0f);
	UFUNCTION(BlueprintCallable, Category = "ALS|Character", Meta = (ReturnDisplayName = "Success"))
	bool StartMantlingGrounded();
	UFUNCTION(BlueprintCallable, Category = "ALS|Character")
	void StartRagdolling();
	UFUNCTION(BlueprintCallable, Category = "ALS|Character", Meta = (ReturnDisplayName = "Success"))
	bool StopRagdolling();
	
protected:
	//////////////////////////////////
	/// Protected K2 Events
	//////////////////////////////////
	UFUNCTION(BlueprintImplementableEvent, Category = "Als Character")
	void K2_OnLocomotionModeChanged(const FGameplayTag& PreviousLocomotionMode);
	UFUNCTION(BlueprintImplementableEvent, Category = "Als Character")
	void K2_OnDesiredAimingChanged(bool bPreviousDesiredAiming);
	UFUNCTION(BlueprintImplementableEvent, Category = "Als Character")
	void K2_OnRotationModeChanged(const FGameplayTag& PreviousRotationMode);
	UFUNCTION(BlueprintImplementableEvent, Category = "Als Character")
	void K2_OnStanceChanged(const FGameplayTag& PreviousStance);
	UFUNCTION(BlueprintImplementableEvent, Category = "Als Character")
	void K2_OnGaitChanged(const FGameplayTag& PreviousGait);
	UFUNCTION(BlueprintImplementableEvent, Category = "Als Character")
	void K2_OnOverlayModeChanged(const FGameplayTag& PreviousOverlayMode);
	UFUNCTION(BlueprintImplementableEvent, Category = "Als Character")
	void K2_OnLocomotionActionChanged(const FGameplayTag& PreviousLocomotionAction);
	UFUNCTION(BlueprintImplementableEvent, Category = "Als Character")
	void K2_OnMantlingStarted(const FAlsMantlingParameters& Parameters);
	UFUNCTION(BlueprintImplementableEvent, Category = "Als Character")
	void K2_OnMantlingEnded();
	UFUNCTION(BlueprintImplementableEvent, Category = "Als Character")
	void K2_OnRagdollingStarted();
	UFUNCTION(BlueprintImplementableEvent, Category = "Als Character")
	void K2_OnRagdollingEnded();
	UFUNCTION(BlueprintImplementableEvent, Category = "Als Character")
	UAlsMantlingSettings* K2_SelectMantlingSettings(EAlsMantlingType MantlingType);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Als Character")
	UAnimMontage* SelectGetUpMontage(bool bRagdollFacedUpward);
	
	//////////////////////////////////
	/// Protected Virtuals
	//////////////////////////////////
	virtual void NotifyLocomotionModeChanged(const FGameplayTag& PreviousLocomotionMode);
	virtual void NotifyLocomotionActionChanged(const FGameplayTag& PreviousLocomotionAction);
	virtual void ApplyDesiredStance();
	virtual bool CanSprint() const;
	
	//////////////////////////////////
	/// Empty Virtuals
	//////////////////////////////////
	virtual bool RefreshCustomGroundedMovingRotation(float DeltaTime) { return false; }
	virtual bool RefreshCustomGroundedNotMovingRotation(float DeltaTime) { return false; }
	virtual bool RefreshCustomInAirRotation(float DeltaTime) { return false; }
	
	//////////////////////////////////
	/// Protected Setters
	//////////////////////////////////
	void SetRotationMode(const FGameplayTag& NewRotationMode);
	void SetStance(const FGameplayTag& NewStance);
	void SetLocomotionMode(const FGameplayTag& NewLocomotionMode);
	void SetGait(const FGameplayTag& NewGait);
	void SetInputDirection(FVector NewInputDirection);

	//////////////////////////////////
	/// Protected Refresh Virtual
	//////////////////////////////////
	virtual void RefreshInput(float DeltaTime);
	
private:
	//////////////////////////////////
	/// Private Setters
	//////////////////////////////////
	FORCEINLINE void SetDesiredVelocityYawAngle(float NewDesiredVelocityYawAngle) { COMPARE_ASSIGN_AND_MARK_PROPERTY_DIRTY(ThisClass, DesiredVelocityYawAngle, NewDesiredVelocityYawAngle, this); }
	void SetRagdollTargetLocation(const FVector& NewTargetLocation);
	
	//////////////////////////////////
	/// Private Calculations
	//////////////////////////////////
	FGameplayTag CalculateMaxAllowedGait() const;
	FGameplayTag CalculateActualGait(const FGameplayTag& MaxAllowedGait) const;
	float CalculateGroundedMovingRotationInterpolationSpeed() const;
	float CalculateMantlingStartTime(const UAlsMantlingSettings* MantlingSettings, float MantlingHeight) const;

	//////////////////////////////////
	/// Private Functions
	//////////////////////////////////
	void OnJumpedNetworked();
	void ApplyRotationYawSpeedAnimationCurve(float DeltaTime);
	
	void StartRolling(float PlayRate, float TargetYawAngle);

	bool StartMantlingInAir();
	bool StartMantling(const FAlsMantlingTraceSettings& TraceSettings);
	void StopMantling(bool bStopMontage = false);
	
	void StartRollingImplementation(UAnimMontage* Montage, float PlayRate, float InitialYawAngle, float TargetYawAngle);
	void StartMantlingImplementation(const FAlsMantlingParameters& Parameters);
	
	void StartRagdollingImplementation();
	void StopRagdollingImplementation();

protected:
	/////////////////////////////////
	/// Transient Gameplay Tags
	/////////////////////////////////
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character", Transient)
	FGameplayTag LocomotionMode{AlsLocomotionModeTags::Grounded};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character", Transient)
	FGameplayTag RotationMode{AlsRotationModeTags::ViewDirection};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character", Transient)
	FGameplayTag Stance{AlsStanceTags::Standing};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character", Transient)
	FGameplayTag Gait{AlsGaitTags::Walking};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character", Transient)
	FGameplayTag LocomotionAction;

	/////////////////////////////////
	/// Transient Move Structs
	/////////////////////////////////
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character", Transient)
	FAlsMovementBaseState MovementBase;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character", Transient)
	FAlsViewState ViewState;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character", Transient)
	FAlsLocomotionState LocomotionState;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character", Transient)
	FAlsMantlingState MantlingState;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character", Transient)
	FAlsRagdollingState RagdollingState;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character", Transient)
	FAlsRollingState RollingState;

	/////////////////////////////////
	/// Anim BP and MovementComp
	/////////////////////////////////
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character", Transient, Meta = (ShowInnerProperties))
	TWeakObjectPtr<UAlsAnimationInstance> AnimationInstance;
	UPROPERTY(BlueprintReadOnly, Category = "CUSTOM")
	TObjectPtr<UAlsCharacterMovementComponent> AlsCharacterMovement;

	/////////////////////////////////
	/// Core Settings
	/////////////////////////////////
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character")
	TObjectPtr<UAlsCharacterSettings> Settings;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character")
	TObjectPtr<UAlsMovementSettings> MovementSettings;

	FTimerHandle BrakingFrictionFactorResetTimer;

#pragma region Networking
private:
	/////////////////////////////////
	/// Networking
	/////////////////////////////////
	UFUNCTION(Server, Reliable)
	void ServerStartMantling(const FAlsMantlingParameters& Parameters);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartMantling(const FAlsMantlingParameters& Parameters);
	UFUNCTION(Server, Unreliable)
	void ServerSetRagdollTargetLocation(const FVector_NetQuantize& NewTargetLocation);
	UFUNCTION(Server, Reliable)
	void ServerStopRagdolling();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopRagdolling();
	UFUNCTION(Server, Reliable)
	void ServerStartRagdolling();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartRagdolling();
	UFUNCTION(Server, Reliable)
	void ServerStartRolling(UAnimMontage* Montage, float PlayRate, float InitialYawAngle, float TargetYawAngle);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartRolling(UAnimMontage* Montage, float PlayRate, float InitialYawAngle, float TargetYawAngle);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnJumpedNetworked();
	UFUNCTION(Server, Unreliable)
	void ServerSetReplicatedViewRotation(const FRotator& NewViewRotation);
	UFUNCTION(Client, Reliable)
	void ClientSetDesiredGait(const FGameplayTag& NewDesiredGait);
	UFUNCTION(Server, Reliable)
	void ServerSetDesiredGait(const FGameplayTag& NewDesiredGait);
	UFUNCTION(Client, Reliable)
	void ClientSetOverlayMode(const FGameplayTag& NewOverlayMode);
	UFUNCTION(Server, Reliable)
	void ServerSetOverlayMode(const FGameplayTag& NewOverlayMode);
	UFUNCTION(Client, Reliable)
	void ClientSetDesiredAiming(bool bNewDesiredAiming);
	UFUNCTION(Server, Reliable)
	void ServerSetDesiredAiming(bool bNewDesiredAiming);
	UFUNCTION(Client, Reliable)
	void ClientSetViewMode(const FGameplayTag& NewViewMode);
	UFUNCTION(Server, Reliable)
	void ServerSetViewMode(const FGameplayTag& NewViewMode);
	UFUNCTION(Client, Reliable)
	void ClientSetDesiredRotationMode(const FGameplayTag& NewDesiredRotationMode);
	UFUNCTION(Server, Reliable)
	void ServerSetDesiredRotationMode(const FGameplayTag& NewDesiredRotationMode);
	UFUNCTION(Client, Reliable)
	void ClientSetDesiredStance(const FGameplayTag& NewDesiredStance);
	UFUNCTION(Server, Reliable)
	void ServerSetDesiredStance(const FGameplayTag& NewDesiredStance);
	
	//////////////////////////////////
	/// Private OnReplicated
	//////////////////////////////////
	UFUNCTION()
	void OnReplicated_DesiredAiming(bool bPreviousDesiredAiming);
	UFUNCTION()
	void OnReplicated_OverlayMode(const FGameplayTag& PreviousOverlayMode);
	UFUNCTION()
	void OnReplicated_ReplicatedViewRotation();

	//////////////////////////////////
	/// Private Setters (typically RPC)
	//////////////////////////////////
	void SetViewMode(const FGameplayTag& NewViewMode, bool bSendRpc);
	void SetDesiredAiming(bool bNewDesiredAiming, bool bSendRpc);
	void SetDesiredRotationMode(const FGameplayTag& NewDesiredRotationMode, bool bSendRpc);
	void SetDesiredStance(const FGameplayTag& NewDesiredStance, bool bSendRpc);
	void SetDesiredGait(const FGameplayTag& NewDesiredGait, bool bSendRpc);
	void SetOverlayMode(const FGameplayTag& NewOverlayMode, bool bSendRpc);
	void SetReplicatedViewRotation(const FRotator& NewViewRotation, bool bSendRpc);

protected:
	//////////////////////////////////
	/// Protected Replicated Vars
	//////////////////////////////////
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character", Transient, Replicated)
	FVector_NetQuantize RagdollTargetLocation;
	// Replicated raw view rotation. Depending on the context, this rotation can be in world space, or in movement
	// base space. In most cases, it is better to use FAlsViewState::Rotation to take advantage of network smoothing.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character", Transient, ReplicatedUsing = "OnReplicated_ReplicatedViewRotation")
	FRotator ReplicatedViewRotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character", Transient, Replicated)
	FVector_NetQuantizeNormal InputDirection;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character",	Transient, Replicated, Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float DesiredVelocityYawAngle;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character|Desired State", ReplicatedUsing = "OnReplicated_DesiredAiming")
	bool bDesiredAiming;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character|Desired State", Replicated)
	FGameplayTag DesiredRotationMode{AlsRotationModeTags::ViewDirection};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character|Desired State", Replicated)
	FGameplayTag DesiredStance{AlsStanceTags::Standing};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character|Desired State", Replicated)
	FGameplayTag DesiredGait{AlsGaitTags::Running};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character|Desired State", Replicated)
	FGameplayTag ViewMode{AlsViewModeTags::ThirdPerson};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CUSTOM|Als Character|Desired State", ReplicatedUsing = "OnReplicated_OverlayMode")
	FGameplayTag OverlayMode{AlsOverlayModeTags::Default};
#pragma endregion Networking

#pragma region Debug
public:
	//////////////////////////////////
	/// Debug
	//////////////////////////////////
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& Unused, float& VerticalLocation) override;
private:
	static void DisplayDebugHeader(const UCanvas* Canvas, const FText& HeaderText, const FLinearColor& HeaderColor, float Scale, float HorizontalLocation, float& VerticalLocation);
	void DisplayDebugCurves(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;
	void DisplayDebugState(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;
	void DisplayDebugShapes(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;
	void DisplayDebugTraces(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;
	void DisplayDebugMantling(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;
#pragma endregion Debug

#pragma region PrivateRefreshes
private:
	//////////////////////////////////
	/// Private Refreshes
	//////////////////////////////////
	void RefreshMeshProperties() const;
	void RefreshMovementBase();
	void RefreshRotationMode();
	void RefreshGait();
	void RefreshView(float DeltaTime);
	void RefreshViewNetworkSmoothing(float DeltaTime);
	void RefreshLocomotionLocationAndRotation();
	void RefreshLocomotionEarly();
	void RefreshLocomotion(float DeltaTime);
	void RefreshLocomotionLate(float DeltaTime);
	void RefreshGroundedAimingRotation(float DeltaTime);
	bool RefreshConstrainedAimingRotation(float DeltaTime, bool bApplySecondaryConstraint = false);
	void RefreshGroundedRotation(float DeltaTime);
	void RefreshInAirRotation(float DeltaTime);
	void RefreshInAirAimingRotation(float DeltaTime);
	void RefreshRotation(float TargetYawAngle, float DeltaTime, float RotationInterpolationSpeed);
	void RefreshRotationExtraSmooth(float TargetYawAngle, float DeltaTime, float RotationInterpolationSpeed, float TargetYawAngleRotationSpeed);
	void RefreshRotationInstant(float TargetYawAngle, ETeleportType Teleport = ETeleportType::None);
	FORCEINLINE void RefreshTargetYawAngleUsingLocomotionRotation() { RefreshTargetYawAngle(UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw)); }
	void RefreshTargetYawAngle(float TargetYawAngle);
	void RefreshViewRelativeTargetYawAngle();
	void RefreshRolling(float DeltaTime);
	void RefreshRollingPhysics(float DeltaTime);
	void RefreshMantling();
	void RefreshRagdolling(float DeltaTime);
	void RefreshRagdollingActorTransform(float DeltaTime);
#pragma endregion PrivateRefreshes

#pragma region PublicInline
public:
	//////////////////////////////////
	/// Inline Public Setters
	//////////////////////////////////
	UFUNCTION(BlueprintCallable, Category = "ALS|Character")
	FORCEINLINE void SetDesiredAiming(bool bNewDesiredAiming) { SetDesiredAiming(bNewDesiredAiming, true); }
	UFUNCTION(BlueprintCallable, Category = "ALS|Character", Meta = (AutoCreateRefTerm = "NewViewMode"))
	FORCEINLINE void SetViewMode(const FGameplayTag& NewViewMode) { SetViewMode(NewViewMode, true); }
	UFUNCTION(BlueprintCallable, Category = "ALS|Character", Meta = (AutoCreateRefTerm = "NewDesiredRotationMode"))
	FORCEINLINE void SetDesiredRotationMode(const FGameplayTag& NewDesiredRotationMode) { SetDesiredRotationMode(NewDesiredRotationMode, true); }
	UFUNCTION(BlueprintCallable, Category = "ALS|Character", Meta = (AutoCreateRefTerm = "NewDesiredStance"))
	FORCEINLINE void SetDesiredStance(const FGameplayTag& NewDesiredStance) { SetDesiredStance(NewDesiredStance, true); }
	UFUNCTION(BlueprintCallable, Category = "ALS|Character", Meta = (AutoCreateRefTerm = "NewDesiredGait"))
	FORCEINLINE void SetDesiredGait(const FGameplayTag& NewDesiredGait) { SetDesiredGait(NewDesiredGait, true); }
	UFUNCTION(BlueprintCallable, Category = "ALS|Character", Meta = (AutoCreateRefTerm = "NewOverlayMode"))
	FORCEINLINE void SetOverlayMode(const FGameplayTag& NewOverlayMode) { SetOverlayMode(NewOverlayMode, true); }
	
	//////////////////////////////////
	/// Inline Public Getters
	//////////////////////////////////
	FORCEINLINE const FGameplayTag& GetViewMode() const { return ViewMode; }
	FORCEINLINE const FGameplayTag& GetLocomotionMode() const { return LocomotionMode; }
	FORCEINLINE const FGameplayTag& GetDesiredRotationMode() const { return DesiredRotationMode; }
	FORCEINLINE const FGameplayTag& GetRotationMode() const { return RotationMode; }
	FORCEINLINE const FGameplayTag& GetDesiredStance() const { return DesiredStance; }
	FORCEINLINE const FGameplayTag& GetStance() const { return Stance; }
	FORCEINLINE const FGameplayTag& GetDesiredGait() const { return DesiredGait; }
	FORCEINLINE const FGameplayTag& GetGait() const { return Gait; }
	FORCEINLINE const FGameplayTag& GetOverlayMode() const { return OverlayMode; }
	FORCEINLINE const FGameplayTag& GetLocomotionAction() const { return LocomotionAction; }
	FORCEINLINE const FVector& GetInputDirection() const { return InputDirection; }
	FORCEINLINE const FAlsViewState& GetViewState() const { return ViewState; }
	FORCEINLINE const FAlsLocomotionState& GetLocomotionState() const { return LocomotionState; }
	FORCEINLINE bool IsDesiredAiming() const { return bDesiredAiming; }
#pragma endregion PublicInline
};