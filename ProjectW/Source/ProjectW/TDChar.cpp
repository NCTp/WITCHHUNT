// Fill out your copyright notice in the Description page of Project Settings.


#include "TDChar.h"
#include "PaperFlipbookComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "ProjectWGameMode.h"

#define PrintString(String) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, String)

ATDChar::ATDChar()
{
	//PlayerController = CreateDefaultSubobject<APlayerController>(TEXT("Player Movement"));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

}

void ATDChar::BeginPlay()
{
	Super::BeginPlay();
	TDCharState = ETDCharStates::TDCharState_Idle;

	// Privates Member variables
	RightValue = 0.0f;
	UpValue = 0.0f;
	m_bisCanMove = true;
	m_bisDefault = true;
	m_bisFront = false;
	m_bisSide = false;
	m_bisBack = false;
	m_bisRight = true;
	m_bisDashStart = false;
	m_bisDashEnd = false;
	m_LockDirection = false;
	m_bisAttacking = false;
	m_bisFirstAttack = false;
	m_bisLastAttack = false;
	m_bisAttackFront = true;
	m_bisAttackBack = false;
	m_bisAttackSide = false;
	m_attackCounts = 0;
	PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = true;
		PlayerController->bEnableClickEvents = true;
		PlayerController->bEnableMouseOverEvents = true;
	}

	
}

void ATDChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateAnimation();

	// Flip sprites
	if (RightValue > 0 && !m_bisRight && !m_bisAttacking)
		Flip();
	else if (RightValue < 0 && m_bisRight && !m_bisAttacking)
		Flip();


	if (m_bisDashStart && !m_bisDashEnd && !m_bisAttacking)
	{
		//PrintString(TEXT("Dashing"));
		FVector myVelocity;
		FVector myPosition;
		if (!m_LockDirection)
		{
			myVelocity = this->GetVelocity() * m_Walkspeed * 15.0f;
		}
		myPosition = this->GetActorLocation();
		this->SetActorLocation(myPosition + myVelocity * DeltaTime);

		/*
		AActor* dashEffect = nullptr;
		dashEffect = GetWorld()->SpawnActor<AActor>(DashEffect,
			GetActorLocation(), GetActorRotation(),
			TDCharSpawnInfo);
			*/

	}
	

}

void ATDChar::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAxis("MoveRight", this, &ATDChar::MoveRight);
	PlayerInputComponent->BindAxis("MoveUp", this, &ATDChar::MoveUp);
	PlayerInputComponent->BindAction("Roll", IE_Pressed, this, &ATDChar::Dash);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ATDChar::MeleeAttack);

}
/*
* Right or Left input
* side값에 대한 정보와 sprite flip 여부를 결정함
*/
void ATDChar::MoveRight(float Value)
{
	RightValue = Value;
	
	if (m_bisCanMove && !m_bisAttacking)
	{
		FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
		AddMovementInput(Direction, Value);


		if (Value > 0 && m_bisDefault)
		{
			SetState(ETDCharStates::TDCharState_Run);
			m_bisDefault = false;
			//m_bisFront = false;
			//m_bisBack = false;
			m_bisSide = true;

		}
		else if (Value < 0 && m_bisDefault)
		{
			SetState(ETDCharStates::TDCharState_Run);
			m_bisDefault = false;
			//m_bisFront = false;
			//m_bisBack = false;
			m_bisSide = true;

		}
		else if (Value == 0 && !m_bisFront && !m_bisBack)
		{
			SetState(ETDCharStates::TDCharState_Idle);
			m_bisDefault = true;
			m_bisSide = false;
		}
	}
	
}
/*
* Front or back input
* front, back값에 대한 정보를 정함
*/
void ATDChar::MoveUp(float Value)
{
	UpValue = Value;
	if (m_bisCanMove && !m_bisAttacking)
	{
		//SetState(ETDCharStates::TDCharState_Run);
		
		if (Value > 0 && m_bisDefault && !m_bisFront)
		{
			SetState(ETDCharStates::TDCharState_Run);
			// Front 가 True
			m_bisDefault = false;
			m_bisFront = true;
			m_bisBack = false;
			//m_bisSide = false;
			PrintString(TEXT("Front"));
		}
		else if (Value < 0 && m_bisDefault && !m_bisBack)
		{
			SetState(ETDCharStates::TDCharState_Run);
			// Front 가 False
			m_bisDefault = false;
			m_bisFront = false;
			m_bisBack = true;
			//m_bisSide = false;
			PrintString(TEXT("Back"));
		}
		
		if (Value == 0 && !m_bisSide)
		{
			SetState(ETDCharStates::TDCharState_Idle);

			m_bisDefault = true;
			m_bisFront = false;
			m_bisBack = false;

			//PrintString(TEXT("No Movement"));
		}
		// Find out which way is "right" and record that the player wants to move that way.
		
		FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
	
}

/*
* Dash Function
* first checking dash is started or not
* change the states and do dash
*/
void ATDChar::Dash()
{
	if (!m_bisDashStart && !m_bisDashEnd && !m_bisAttacking && m_MP >= 0.0f)
	{
		AActor* dashEffect = nullptr;
		dashEffect = GetWorld()->SpawnActor<AActor>(DashEffect,
			GetActorLocation(), GetActorRotation(),
			TDCharSpawnInfo);
		
		if (m_MP >= 30.0f)
			m_MP -= 30.0f;
		else if (m_MP < 30.0f)
			m_MP = 0.0f;

		m_bisDashStart = true;
		m_LockDirection = true;
		SetState(ETDCharStates::TDCharState_Dash);
		FTimerHandle DashWaitHandle;
		float WaitTime = 0.2f;
		GetWorld()->GetTimerManager().SetTimer(DashWaitHandle, FTimerDelegate::CreateLambda([&]()
			{
				m_bisCanMove = false;
				m_bisDashStart = false;
				m_bisDashEnd = true;
				m_LockDirection = false;
				//PrintString(TEXT("First Waiting"));
				FTimerHandle IdleWaitHandle;
				float WaitTime_2 = 0.4f;
				GetWorld()->GetTimerManager().SetTimer(IdleWaitHandle, FTimerDelegate::CreateLambda([&]()
					{
						SetState(ETDCharStates::TDCharState_Run);
						m_bisCanMove = true;
						m_bisDashStart = false;
						m_bisDashEnd = false;
						//PrintString("Second Waiting");

					}), WaitTime_2, false);
			}), WaitTime, false); 

	}

	
}
/*
*  Melee Attack Function
*  현재 문제 = 마우스 방향으로 나가도록 구현을 안했음. 수정 필요 -> 수정 완료
*/
void ATDChar::MeleeAttack()
{
	// Check Mouse Position at Game World
	FHitResult TraceHitResult;

	PlayerController->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
	FVector MouseWorldLocation = TraceHitResult.Location;

	FVector AttackDirection = MouseWorldLocation - GetActorLocation();
	FVector nAttackDirection = AttackDirection;
	nAttackDirection.Normalize();
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("%f, %f"), MouseWorldLocation.X, MouseWorldLocation.Y));
	FVector SpawnLocation;
	FRotator SpawnRotator;
	// Check Mouse Position at Viewport (not in Game World)
	PlayerController->GetMousePosition(m_MouseXValue, m_MouseYValue);
	// Check Viewport Size and Center
	FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	const FVector2D  ViewportCenter = FVector2D(ViewportSize.X / 2, ViewportSize.Y / 2);

	// Check Up/Down due to Mouse Position and Setup SpawnRotator
	if (AttackDirection.Y > 0 && AttackDirection.X < 125 && AttackDirection.X > -125)
	{
		PrintString("Attack Front");
		m_bisAttackFront = true;
		m_bisAttackBack = false;
		m_bisAttackSide = false;
		SpawnRotator = FRotator(0.0f, 90.0f, 0.0f);
	}
	else if (AttackDirection.Y <= 0 && AttackDirection.X < 125 && AttackDirection.X > -125)
	{
		PrintString("Attack Back");
		m_bisAttackFront = false;
		m_bisAttackBack = true;
		m_bisAttackSide = false;
		SpawnRotator = FRotator(0.0f, -90.0f, 0.0f);
	}
	else if (AttackDirection.X >= 125 || AttackDirection.X <= -125)
	{
		if (AttackDirection.X >= 125 && !m_bisRight)
			Flip();
		else if (AttackDirection.X <= -125 && m_bisRight)
			Flip();
		PrintString("Attack Side");
		m_bisAttackFront = false;
		m_bisAttackBack = false;
		m_bisAttackSide = true;
		
		if (AttackDirection.X >= 125)
			SpawnRotator = FRotator(0.0f, 0.0f, 0.0f);
		else if (AttackDirection.X <= -125)
			SpawnRotator = FRotator(0.0f, 180.0f, 0.0f);
	}

	// Player Attack Implements
	APlayerMeleeProjectile* projectile = nullptr;
	TDCharSpawnInfo.Owner = this;
	/*
	if (!m_bisFirstAttack&& !m_bisLastAttack)
	{
		projectile = GetWorld()->SpawnActor<APlayerMeleeProjectile>(MeleeProjectile,
			GetActorLocation() - nAttackDirection * 10, SpawnRotator,
			TDCharSpawnInfo);
	}
	else if (m_bisFirstAttack && !m_bisLastAttack)
	{
		projectile = GetWorld()->SpawnActor<APlayerMeleeProjectile>(MeleeProjectile,
			GetActorLocation() - nAttackDirection * 10, SpawnRotator,
			TDCharSpawnInfo);
	}
	*/
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("%f"), SpawnRotator.Yaw));
	FMath::DegreesToRadians(MouseWorldLocation.X/MouseWorldLocation.Y);
	SetState(ETDCharStates::TDCharState_MeleeAttack);

	m_bisAttacking = true;
	/*
	if (m_attackCounts == 0)
	{
		m_attackCounts++;
		// 0.7초 후에 공격 초기화
		projectile = GetWorld()->SpawnActor<APlayerMeleeProjectile>(MeleeProjectile,
			GetActorLocation() - nAttackDirection * 10, SpawnRotator,
			TDCharSpawnInfo);
		FTimerHandle AttackWaitHandle;
		float AttackWaitTime = 0.7f;
		GetWorld()->GetTimerManager().SetTimer(AttackWaitHandle, FTimerDelegate::CreateLambda([&]()
			{
				m_bisFirstAttack = false;
				m_bisLastAttack = false;
				m_bisAttacking = false;
				SetState(ETDCharStates::TDCharState_Run);
				//PrintString(TEXT("Attack End1"));

			}), AttackWaitTime, false);
	}
	else if (m_attackCounts == 1)
	{
		m_attackCounts++;
		projectile = GetWorld()->SpawnActor<APlayerMeleeProjectile>(MeleeProjectile,
			GetActorLocation() - nAttackDirection * 10, SpawnRotator,
			TDCharSpawnInfo);
		
	}
	*/

	if (!m_bisFirstAttack && !m_bisLastAttack)
	{
		
		m_bisFirstAttack = true;
		m_bisLastAttack = false;
		// 0.7초 후에 공격 초기화
		projectile = GetWorld()->SpawnActor<APlayerMeleeProjectile>(MeleeProjectile,
			GetActorLocation() - nAttackDirection * 10, SpawnRotator,
			TDCharSpawnInfo);
		FTimerHandle AttackWaitHandle;
		float AttackWaitTime = 0.7f;
		GetWorld()->GetTimerManager().SetTimer(AttackWaitHandle, FTimerDelegate::CreateLambda([&]()
			{
				m_bisFirstAttack = false;
				m_bisLastAttack = false;
				m_bisAttacking = false;
				SetState(ETDCharStates::TDCharState_Run);
				//PrintString(TEXT("Attack End1"));

			}), AttackWaitTime, false);
	}
	else if (m_bisFirstAttack && !m_bisLastAttack)
	{
		projectile = GetWorld()->SpawnActor<APlayerMeleeProjectile>(MeleeProjectile,
			GetActorLocation() - nAttackDirection * 10, SpawnRotator,
			TDCharSpawnInfo);
		m_bisFirstAttack = false;
		m_bisLastAttack = true;

	}
}
void ATDChar::RangeAttack()
{

}
/*
*  Get Damage Function
*  Decrease Health when get attacked
*  We will call this function in blueprint editor.
*/
void ATDChar::GetDamage(float damage)
{
	m_HP -= damage;
	PrintString(TEXT("health down"));
}
/*
*  Get HP Function
*  Return HP
*  We will call this function in blueprint editor.
*/
float ATDChar::GetHP()
{
	return m_HP;
}
/*
*  Get MP Function
*  Return MP
*  We will call this function in blueprint editor.
*/
float ATDChar::GetMP()
{
	return m_MP;
}
/*
*  Set HP Function
*  Set HP
*  We will call this function in blueprint editor.
*/
void ATDChar::SetHP(float HP)
{
	if (m_HP < 100)
		m_HP += HP;
	else if (m_HP >= 100)
		m_HP = 100.0f;

}
/*
*  Set Mp Function
*  Set MP
*  We will call this function in blueprint editor.
*/
void ATDChar::SetMP(float MP)
{
	if (m_MP < 100)
		m_MP += MP;
	else if (m_MP >= 100)
		m_MP = 100.0f;
}

/*
*  UpdateAnimation
*  Update Animation Every frame
*  
*/
void ATDChar::UpdateAnimation()
{
	UPaperFlipbook* anim = nullptr;
	
	switch (TDCharState)
	{

	case (ETDCharStates::TDCharState_Idle):
		if (m_bisFront || m_bisDefault)
			anim = Front_TDIdleAnim;
		else if (m_bisSide)
			anim = Side_TDIdleAnim;
		else if (m_bisBack)
			anim = Back_TDIdleAnim;
		break;
	case (ETDCharStates::TDCharState_Run):
		if (m_bisFront)
			anim = Front_TDRunAnim;
		else if (m_bisSide)
			anim = Side_TDRunAnim;
		else if (m_bisBack)
			anim = Back_TDRunAnim;
		break;
	case (ETDCharStates::TDCharState_Dash):
		/**/
		if (m_bisDashStart && !m_bisDashEnd)
			anim = TDDashStartAnim;
		else if (!m_bisDashStart && m_bisDashEnd)
			anim = TDDashEndAnim;
		break;
	case (ETDCharStates::TDCharState_MeleeAttack):
		if (m_bisAttackFront)
		{
			if (m_bisFirstAttack && !m_bisLastAttack)
				anim = Front_TDMeleeAttackAnim_1;
			else if (!m_bisFirstAttack && m_bisLastAttack) {
				anim = Front_TDMeleeAttackAnim_2;
				
			}
			break;
		}
		else if (m_bisAttackBack)
		{
			if (m_bisFirstAttack && !m_bisLastAttack)
				anim = Back_TDMeleeAttackAnim_1;
			else if (!m_bisFirstAttack && m_bisLastAttack)
				anim = Back_TDMeleeAttackAnim_2;
			break;
		}
		else if (m_bisAttackSide)
		{
			if (m_bisFirstAttack && !m_bisLastAttack)
				anim = Side_TDMeleeAttackAnim_1;
			else if (!m_bisFirstAttack && m_bisLastAttack)
				anim = Side_TDMeleeAttackAnim_2;
			break;
		}
	}

	GetSprite()->SetFlipbook(anim);
	GetSprite()->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	
}


void ATDChar::SetState(ETDCharStates NewState)
{
	if (TDCharState != ETDCharStates::TDCharState_Dead)
		TDCharState = NewState;
}

void ATDChar::Flip()
{
	FVector scale = GetSprite()->GetRelativeScale3D();
	scale.X *= -1;
	//SetActorScale3D(scale);
	GetSprite()->SetRelativeScale3D(scale);
	m_bisRight = !m_bisRight;
}
