// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicHUD.h"
#include "Engine/Canvas.h"
#include "Engine/StreamableManager.h"
#include "Engine/Texture2D.h"



void ABasicHUD::DrawHUD()
{
	Super::DrawHUD();

	//ĵ������������
	//Canvas->Get

	//���׸���
	//Draw2DLine(100, 100, 200, 200, FColor::Red);

	//�β��� ����
	//DrawLine(300, 300, 400, 400, FLinearColor::Green, 3.0f);

	//DrawTextureSimple()

	if (Crosshair) {
		DrawTextureSimple(Crosshair, 
			Canvas->SizeX / 2 - (Crosshair->GetSizeX() / 2),
			Canvas->SizeY / 2 - (Crosshair->GetSizeY() / 2));
	}
}

void ABasicHUD::BeginPlay()
{
	Super::BeginPlay();
	FStreamableManager loader;
	
	//��� �����ϱ�
	T_Crosshair.SetPath(TEXT("Texture2D'/Game/UI/Crosshair/crosshair.crosshair'"));

	//�����ϱ�
	//���� �ε�, �ε��� ȭ���� ����, 10�� �Ž� �ε� ���������� ����
	//Crosshair = loader.LoadSynchronous<UTexture2D>(T_Crosshair);

	//�񵿱� �ε�
	loader.RequestAsyncLoad(T_Crosshair, FStreamableDelegate::CreateUObject(this, &ABasicHUD::LoadHUDTexture));
	
}

void ABasicHUD::LoadHUDTexture() {
	//�ε�õ�
	Crosshair = Cast<UTexture2D>(T_Crosshair.TryLoad());
}