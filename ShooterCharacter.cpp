// Fill out your copyright notice in the Description page of Project Settings.

#include "Engine/World.h"
#include "Grenade.h"
#include "SimpleShooterGameModeBase.h"
#include "Components/CapsuleComponent.h"
#include "ShooterCharacter.h"
#include "Gun.h"

// Sets default values
AShooterCharacter::AShooterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//Capsule = FindComponentByClass<UCapsuleComponent>();
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	GetMesh()->HideBoneByName(TEXT("weapon_r"), EPhysBodyOp::PBO_None);
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AShooterCharacter::OnOverlapBegin);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &AShooterCharacter::OnOverlapEnd);
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//AddGun();
}

bool AShooterCharacter::IsDead() const
{
	if (Health == 0) return true;
	else return false;
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("LookRight"), this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Shoot"), EInputEvent::IE_Pressed, this, &AShooterCharacter::Shoot);
	PlayerInputComponent->BindAction(TEXT("PickUp"), EInputEvent::IE_Pressed, this, &AShooterCharacter::AddGun);
	PlayerInputComponent->BindAction(TEXT("SwitchWeapon"), EInputEvent::IE_Pressed, this, &AShooterCharacter::SwitchGun);
	PlayerInputComponent->BindAction(TEXT("ThrowGrenade"), EInputEvent::IE_Pressed, this, &AShooterCharacter::ThrowGrenade);
}

void AShooterCharacter::MoveForward(float AxisValue)
{
	AddMovementInput(GetActorForwardVector() * AxisValue);
}
void AShooterCharacter::MoveRight(float AxisValue)
{
	AddMovementInput(GetActorRightVector() * AxisValue);
}

void AShooterCharacter::Shoot()
{
	if (EquippedGun())
	{
		EquippedGun()->SetOwner(this);
		//Gun->SetOwner(this);
		EquippedGun()->PullTrigger();
	}
}

float AShooterCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float DamageToApply = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	DamageToApply = FMath::Min(Health, DamageToApply);
	Health -= DamageToApply;
	UE_LOG(LogTemp, Warning, TEXT("Health: %f"), Health)

	if(IsDead())
	{
		ASimpleShooterGameModeBase* GameMode = GetWorld()->GetAuthGameMode<ASimpleShooterGameModeBase>();
		if (GameMode != nullptr)
		{
			GameMode->PawnKilled(this);
		}
		DetachFromControllerPendingDestroy();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	return DamageToApply;
}

float AShooterCharacter::GetHealthPercent() const
{
	return Health/MaxHealth;
}

void AShooterCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != nullptr && OtherActor != this && OtherComp != nullptr)
	{
		if (OtherActor->IsA<AGun>())
		{
			auto Weapon = Cast<AGun>(OtherActor);
			Weapon->OnInteractionRangeEntered(this);
		}
	}
}

void  AShooterCharacter::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->IsA<AGun>())
	{
		auto Weapon = Cast<AGun>(OtherActor);
		Weapon->OnInteractionRangeExited();
	}
}

void AShooterCharacter::SwitchGun()
{
	if (CurrentGunSlot == EWeaponSlot::Primary)
	{
		CurrentGunSlot = EWeaponSlot::Secondary;
		UpdateGunAttachments();
	}
	else if (CurrentGunSlot == EWeaponSlot::Secondary)
	{
		CurrentGunSlot = EWeaponSlot::Primary;
		UpdateGunAttachments();
	}
}

void AShooterCharacter::AddGun()
{
	TArray<AActor*> Actors;
	auto Capsule = FindComponentByClass<UCapsuleComponent>();
	Capsule->GetOverlappingActors(Actors);
	for (AActor* Actor : Actors)
	{
		if (Actor->IsA<AGun>())
		{
			auto GunToEquip = Cast<AGun>(Actor);
			GunToEquip->OnInteracted(this);
			break;
		}
	}
}

void AShooterCharacter::PickUpGun(AGun* gun)
{
	bool IsPrimaryGun = IsValid(PrimaryGun);
	bool IsSecondaryGun = IsValid(SecondaryGun);
	if (!IsPrimaryGun)
	{
		PrimaryGun = gun;
		UpdateGunAttachments();
	}
	else if (IsPrimaryGun)
	{
		if (!IsSecondaryGun)
		{
			SecondaryGun = gun;
			UpdateGunAttachments();
		}
		else if (IsSecondaryGun)
		{
			if (IsValid(EquippedGun()))
			{
				EquippedGun()->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				EquippedGun()->Drop();
				if (CurrentGunSlot == EWeaponSlot::Primary) PrimaryGun = nullptr;
				else if (CurrentGunSlot == EWeaponSlot::Secondary) SecondaryGun = nullptr;
				PickUpGun(gun);
			}
		}
	}
}

void AShooterCharacter::UpdateGunAttachments()
{
	if (IsValid(EquippedGun()))
	{
		//Gun = EquippedGun();
		EquippedGun()->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("WeaponSocket"));
	}
	if (IsValid(StowedGun()))
	{
		auto Stowed = StowedGun();
		StowedGun()->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("GunStowedSocket"));
	}
}

AGun* AShooterCharacter::EquippedGun()
{
	AGun* Weapon = nullptr;
	if (CurrentGunSlot == EWeaponSlot::Primary)
	{
		Weapon = PrimaryGun;
	}
	else if (CurrentGunSlot == EWeaponSlot::Secondary)
	{
		Weapon = SecondaryGun;
	}
	return Weapon;
}

AGun* AShooterCharacter::StowedGun()
{
	AGun* Weapon = nullptr;
	if (CurrentGunSlot == EWeaponSlot::Primary)
	{
		Weapon = SecondaryGun;
	}
	else if (CurrentGunSlot == EWeaponSlot::Secondary)
	{
		Weapon = PrimaryGun;
	}
	return Weapon;
}

void AShooterCharacter::ThrowGrenade()
{
	if (!ensure(GrenadeBlueprint)) return;

	GrenadeOffset.X = 300;
	FVector StartLocation = GetActorLocation() + GetControlRotation().RotateVector(GrenadeOffset);

	//GetWorld()->SpawnActor<AGrenade>(GrenadeBlueprint, StartLocation, GetActorRotation());
	auto Grenade = GetWorld()->SpawnActor<AGrenade>(GrenadeBlueprint, GetMesh()->GetSocketLocation(FName("Grenade")), GetMesh()->GetSocketRotation(FName("Grenade")));
	Grenade->LaunchGrenade(LaunchSpeed);
}
