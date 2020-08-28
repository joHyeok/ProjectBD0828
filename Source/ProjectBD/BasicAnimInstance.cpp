// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicAnimInstance.h"
#include "BasicPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
void UBasicAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	ABasicPlayer* Pawn = Cast<ABasicPlayer>(TryGetPawnOwner());
	//��ȿ�˻�
	if (Pawn && Pawn->IsValidLowLevel()) {
		//������ ����Ѵ�.
		Direction = CalculateDirection(Pawn->GetCharacterMovement()->Velocity, Pawn->GetActorRotation());

		//ũ�⸸ �˾Ƶ��ȴٸ� SizeSquared()�� �������.
		Speed = Pawn->GetCharacterMovement()->Velocity.Size();

		bIsCrouch = Pawn->bIsCrouched;
		bIsFire = Pawn->bIsFire;
		bIsIronsight = Pawn->bIsIronsight;
		bIsSprint = Pawn->bIsSprint;
	}
}
