// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

class AGun;
class AGrenade;

UENUM()
enum class EWeaponSlot : uint8
{
	Primary,
	Secondary
};

UCLASS()
class SIMPLESHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure)
	bool IsDead() const;

	UFUNCTION(BlueprintPure)
	float GetHealthPercent() const;

	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void Shoot();
	void AddGun();
	void UpdateGunAttachments();
	void PickUpGun(AGun* gun);
	void SwitchGun();
	void ThrowGrenade();

private:

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser);
	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void SetEquippedGun(AGun* Arm);

	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	TSubclassOf<AGrenade> GrenadeBlueprint;

	UPROPERTY(EditDefaultsOnly)
	float MaxHealth = 100;

	UPROPERTY(VisibleAnywhere)
	float Health;

	UPROPERTY(EditDefaultsOnly, Category = "Firing")
	float LaunchSpeed = 100;

	UPROPERTY()
	AGun* Gun = nullptr;

	UPROPERTY()
	AGun* PrimaryGun = nullptr;
	UPROPERTY()
	AGun* SecondaryGun = nullptr;

	FVector GrenadeOffset;

	EWeaponSlot CurrentGunSlot = EWeaponSlot::Primary;

	AGun* EquippedGun();
	AGun* StowedGun();

};
