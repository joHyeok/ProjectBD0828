// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WeaponComponent.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
ABasicPlayer::ABasicPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Weapon = CreateDefaultSubobject<UWeaponComponent>(TEXT("Weapon"));

	SpringArm->SetupAttachment(RootComponent);
	Camera->SetupAttachment(SpringArm);
	Weapon->SetupAttachment(GetMesh(), TEXT("WeaponSocket"));

	GetMesh()->SetRelativeLocation(FVector(0, 0, -GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()));
	//하드코딩
	GetMesh()->SetRelativeRotation(FRotator(0, -90.f, 0));

	//캐릭터 무브먼트에 Navmovement의 CanCrouch 체크하기 하드코딩
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	SpringArm->SocketOffset = FVector(0, 40.0f, 88.0f);
	SpringArm->TargetArmLength = 120.f;
	SpringArm->bUsePawnControlRotation = true;

	GetCharacterMovement()->CrouchedHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();

	NormalSpringArmPosition = SpringArm->GetRelativeLocation();
	CrouchedSpringArmPosition = NormalSpringArmPosition + FVector(0, 0, -GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() / 2);
}

// Called when the game starts or when spawned
void ABasicPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABasicPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABasicPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ABasicPlayer::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ABasicPlayer::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ABasicPlayer::LookUp);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ABasicPlayer::Turn);

	PlayerInputComponent->BindAxis(TEXT("CameraView"), this, &ABasicPlayer::CameraView);
	
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ABasicPlayer::Jump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ABasicPlayer::StopJumping);

	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &ABasicPlayer::Sprint);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &ABasicPlayer::StopSprint);

	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &ABasicPlayer::StartFire);
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Released, this, &ABasicPlayer::StopFire);

	PlayerInputComponent->BindAction(TEXT("Ironsight"), IE_Pressed, this, &ABasicPlayer::StartIronsight);
	PlayerInputComponent->BindAction(TEXT("Ironsight"), IE_Released, this, &ABasicPlayer::StopIronsight);

	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &ABasicPlayer::StartCrouch);
}

void ABasicPlayer::MoveForward(float AxisValue)
{
	//절대 회전, 카메라 같게 설정
	//GetControlRotation().Vector()
	FVector CameraLocation;
	FRotator CameraRotator;
	//컨트롤러가 카메라의 정보를 가지고 있다. 카메라의 위치, 회전값을 저장한다.
	GetController()->GetPlayerViewPoint(CameraLocation, CameraRotator);

	// 캐릭터의 이동을 카메라의 방향으로 이동시킬건지 컨트롤러의 방향으로 이동시킬건지
	// 정해야한다. 스프링암이 폰의 회전값을 가져오는지 안 가져오는지도 설정함으로서
	// 다른 결과가 나온다.

	//카메라의 방향으로 이동한다. 스프링암의 yaw방향으로만 이동한다.
	//FRotator YawBaseRotation = FRotator(0, CameraRotator.Yaw, 0);

	// 컨트롤러의 방향으로 이동한다.
	FRotator YawBaseRotation = FRotator(0, GetControlRotation().Yaw, 0);

	FVector CameraForward = UKismetMathLibrary::GetForwardVector(YawBaseRotation);

	AddMovementInput(CameraForward, AxisValue);
}

void ABasicPlayer::MoveRight(float AxisValue)
{
	FRotator YawBaseRotation = FRotator(0, GetControlRotation().Yaw, 0);
	FVector CameraRight = UKismetMathLibrary::GetRightVector(YawBaseRotation);
	AddMovementInput(CameraRight, AxisValue);
}

void ABasicPlayer::LookUp(float AxisValue)
{
	AddControllerPitchInput(AxisValue);
}

void ABasicPlayer::Turn(float AxisValue)
{
	AddControllerYawInput(AxisValue);
}

void ABasicPlayer::Sprint()
{
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void ABasicPlayer::StopSprint() {
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ABasicPlayer::CameraView(float AxisValue)
{

}

void ABasicPlayer::StartFire()
{
	bIsFire = true;
	OnFire();
}

void ABasicPlayer::StopFire()
{
	bIsFire = false;
}

void ABasicPlayer::OnFire()
{
	//발사가 아니면 return
	if (!bIsFire) {
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	int32 ScreenSizeX;
	int32 ScreenSizeY;
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	FVector CameraLocation;
	FRotator CameraRotation;

	//GetViewportSize : 플레이어가 가진 화면 사이즈
	PC->GetViewportSize(ScreenSizeX, ScreenSizeY);

	//카메라를 관리하는 플레이어 컨트롤러가 가지고 있다.
	//ProjectWorldLocationToScreen : 월드좌표를 화면좌표계로 바꾼다.
	//DeprojectScreenPositionToWorld : 화면 좌표계를 월드 좌표계로 바꾼다. 엔진의 기능임
	//아래 코드는 화면의 가운데 지점을 월드좌표로 가져온거다. 화면의 가운데 즉 에임의 위치다.
	PC->DeprojectScreenPositionToWorld(ScreenSizeX / 2, ScreenSizeY / 2, CrosshairWorldPosition, CrosshairWorldDirection);

	//카메라 위치 가져오기
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	//라인 트레이스 사용
	FVector TraceStart = CameraLocation;
	//끝점은 시작점에서 조준점방향으로 쭉 늘리는것
	FVector TraceEnd = TraceStart + (CrosshairWorldDirection * 99999.f);

	//프젝세팅에 트레이스 체널들의 배열값이다. 그 이넘값을 바이트 형태로 만든것
	//충돌체크할 리스트를 만드는거다.
	TArray <TEnumAsByte<EObjectTypeQuery>> Objects;

	//손으로 하드코딩은 거의 안한다. UPROPERTY()로 빼서 캐릭터 블프에서 사용한다.
	Objects.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
	Objects.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
	Objects.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	//값을 안넣어서 다 맞는걸로 함
	TArray<AActor*> ActorToIgnore;

	//맞은놈 정보 저장
	//온갖 정보를 가지고 있다.
	FHitResult OutHit;

	//충돌하면 true를 반환함
	//월드정보, 시작, 끝, 충돌할 리스트, 스테틱메시의 충돌확인을 false면 단순콜리전으로 할것이냐 true면 복합콜리전으로 할것이냐를 설정한다, 
	//안맞을 애들, 그려진 타입, 맞은 놈 정보, 무시 사항에 폰을 뺄거냐?, 라인색, 맞은 후에 색, 나타날 시간
	//마지막 3개는 보통 안쓴다 보일려면 쓴다
	//카메라에서 쏘는 이유는 UI의 조준선이 화면 기준으로 되어 있기 때문에 에임을 맞춘다는게 카메라에서 조준섬으로 라인을 쏘는걸로 한다.
	//맞는다면 총에서 라인을 쏴서 진짜 맞았는지 확인
	bool Result = UKismetSystemLibrary::LineTraceSingleForObjects(
		GetWorld(), TraceStart, TraceEnd, Objects, true, ActorToIgnore, EDrawDebugTrace::ForDuration, 
		OutHit, true, FLinearColor::Red, FLinearColor::Green, 5.0f);

	//충돌한다면
	if (Result) {
		UE_LOG(LogClass, Warning, TEXT("Hit %s"), *OutHit.GetActor()->GetName());
	}
}

void ABasicPlayer::StartIronsight()
{
	bIsIronsight = true;
}

void ABasicPlayer::StopIronsight()
{
	bIsIronsight = false;
}

void ABasicPlayer::StartCrouch()
{
	//이미 엔진에서 만들어 놓은 함수이다.
	// 앉을 수 있는지 검사해서 앉고 일어서는 걸 수행한다.
	if (CanCrouch()) {
		Crouch();
	}
	else {
		UnCrouch();
	}
}

