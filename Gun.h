// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gun.generated.h"

class AShooterCharacter;

UENUM(BlueprintType)
enum class EGunState : uint8
{
	Dropped,
	Equipped
};

UCLASS()
class SIMPLESHOOTER_API AGun : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGun();

	void PullTrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void OnInteracted(AShooterCharacter* Player);

	UFUNCTION(BlueprintImplementableEvent)
	void OnInteractionRangeEntered(AShooterCharacter* Player);

	UFUNCTION(BlueprintImplementableEvent)
	void OnInteractionRangeExited();

	UFUNCTION(BlueprintImplementableEvent)
	void SetCollision(EGunState NewGunState);

	void Drop();

protected: 
	UPROPERTY(BlueprintReadWrite, Category = "State")
	EGunState GunState; 

private:

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root = nullptr;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* Mesh = nullptr;
	
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* SphereCollision = nullptr;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash = nullptr;

	UPROPERTY(EditAnywhere)
	UParticleSystem* HitFlash = nullptr;

	UPROPERTY(EditAnywhere)
	USoundBase* MuzzleSound = nullptr;

	UPROPERTY(EditAnywhere)
	USoundBase* ImpactSound = nullptr;

	UPROPERTY(EditAnywhere)
	float MaxRange = 1000;

	UPROPERTY(EditAnywhere)
	float Damage = 10;

	bool GunTrace(FHitResult& Hit, FVector& ShotDirection);
	AController* GetOwnerController() const;
	USceneComponent* GetInteractionTextAttached() const;

	void SetGunState(EGunState NewGunState);

};
