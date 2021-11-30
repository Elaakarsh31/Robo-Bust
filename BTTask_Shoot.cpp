// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterCharacter.h"
#include "AIController.h"
#include "BTTask_Shoot.h"

UBTTask_Shoot::UBTTask_Shoot()
{
	NodeName = TEXT("Shoot");
}

EBTNodeResult::Type UBTTask_Shoot::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);
    if (OwnerComp.GetAIOwner() == nullptr) return EBTNodeResult::Failed;

    AShooterCharacter* ShootChar = Cast<AShooterCharacter>(OwnerComp.GetAIOwner()->GetPawn());
    if (ShootChar == nullptr) return EBTNodeResult::Failed;

    ShootChar->Shoot();
    return EBTNodeResult::Succeeded;
}