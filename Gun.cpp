// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "ShooterPlayerController.h"
#include "ShooterCharacter.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Gun.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
AGun::AGun()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	//Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Mesh);
	
	//Root->SetupAttachment(Mesh);
	//Mesh->SetCollisionProfileName("BlockAll"); // v creating a profile would be better v
	//Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
}

// Called when the game starts or when spawned
void AGun::BeginPlay()
{
	Super::BeginPlay();
	SetGunState(EGunState::Dropped);
}

// Called every frame
void AGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGun::PullTrigger()
{
	FHitResult Hit; FVector ShotDirection;
	bool bSuccess = GunTrace(Hit, ShotDirection);
	if (bSuccess)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitFlash, Hit.Location, ShotDirection.Rotation());
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), ImpactSound, Hit.Location, ShotDirection.Rotation());
		FPointDamageEvent DamageEvent(Damage, Hit, ShotDirection, nullptr);
		AActor* HitActor = Hit.GetActor();
		if (HitActor != nullptr)
		{
			AController* OwnerController = GetOwnerController();
			if (OwnerController == nullptr) return;
			HitActor->TakeDamage(Damage, DamageEvent, OwnerController, this);
		}
	}
}

bool AGun::GunTrace(FHitResult& Hit, FVector& ShotDirection)
{
	AController* OwnerController = GetOwnerController();
	if (OwnerController == nullptr) return false;
	UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, Mesh, FName(TEXT("MuzzleFlashSocket")));
	UGameplayStatics::SpawnSoundAttached(MuzzleSound, Mesh, FName(TEXT("MuzzleFlashSocket")));

	FVector Location;
	FRotator Rotation;
	OwnerController->GetPlayerViewPoint(Location, Rotation);
	ShotDirection = -Rotation.Vector();
	FVector End = Location + Rotation.Vector() * MaxRange;

	Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	return GetWorld()->LineTraceSingleByChannel(Hit, Location, End, ECollisionChannel::ECC_GameTraceChannel1, Params);
}

AController* AGun::GetOwnerController() const
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return nullptr;
	return OwnerPawn->GetController();
}

void AGun::SetGunState(EGunState NewGunState)
{
	GunState = NewGunState;
	//SetCollision(GunState);
	auto Sphere = FindComponentByClass<USphereComponent>();
	if (GunState == EGunState::Equipped)
	{
		//SetEquippedCollision();
		if (Sphere) Sphere->SetCollisionProfileName(TEXT("NoCollision"));
		if (Mesh)
		{
			Mesh->SetCollisionProfileName(TEXT("NoCollision"));
			Mesh->SetSimulatePhysics(false);
			Mesh->ResetRelativeTransform();
		}
	}
	else if (GunState == EGunState::Dropped)
	{
		//SetDroppedCollision();
		if (Sphere) Sphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
		if (Mesh)
		{
			Mesh->SetSimulatePhysics(true);
			Mesh-> SetCollisionProfileName(TEXT("DroppedGun"));
		}
	}
}

void AGun::Drop()
{
	SetGunState(EGunState::Dropped);
	//DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Mesh->SetSimulatePhysics(true);
	//Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}

void AGun::OnInteracted(AShooterCharacter* Player)
{
	Mesh->SetSimulatePhysics(false);
	Player->PickUpGun(this);
	SetGunState(EGunState::Equipped);
}

USceneComponent* AGun::GetInteractionTextAttached() const
{
	return Root;
}