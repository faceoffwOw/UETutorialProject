// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBasePawnMovementComponent.h"

void UGCBasePawnMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (ShouldSkipUpdate(DeltaTime))
	{
		return;
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FVector PendingInput = GetPendingInputVector().GetClampedToMaxSize(1.0f);
	Velocity = PendingInput * MaxSpeed;
	ConsumeInputVector();

	if (bEnableGravity)
	{
		FHitResult HitResult;
		FVector StartPoint = UpdatedComponent->GetComponentLocation();
		float TraceDepth = 1.0f;
		float SphereRadius = 50.0f;
		//float LineTraceLength = 50.0f + GetGravityZ() * DeltaTime;
		float LineTraceLength = SphereRadius + TraceDepth;
		FVector EndPoint = StartPoint - TraceDepth * FVector::UpVector;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(GetOwner());

		bool bWasFalling = bIsFalling;
		FCollisionShape Sphere = FCollisionShape::MakeSphere(SphereRadius);
		//bIsFalling = !GetWorld()->LineTraceSingleByChannel(HitResult, StartPoint, EndPoint, ECC_Visibility, CollisionParams);
		bIsFalling = !GetWorld()->SweepSingleByChannel(HitResult, StartPoint, EndPoint, FQuat::Identity, ECC_Visibility, Sphere, CollisionParams);
		
		FString state = bIsFalling ? "True" : "False";
		GEngine->AddOnScreenDebugMessage(1, 2, FColor::Red, FString::Printf(TEXT("State: %s"), *state));

		if (bIsFalling)
		{
			VerticalVelocity += GetGravityZ() * FVector::UpVector * DeltaTime;
			///Velocity += VerticalVelocity;
		}
		else if(bWasFalling && VerticalVelocity.Z < 0.0f)
		{
			VerticalVelocity = FVector::ZeroVector;
		}
	}
	
	Velocity += VerticalVelocity;
	FVector Delta = Velocity * DeltaTime;

	if (!Delta.IsNearlyZero(1e-6f))
	{
		FQuat Rotation = UpdatedComponent->GetComponentQuat();
		//MoveUpdatedComponent(Delta, Rotation, true);

		FHitResult Hit(1.f);
		SafeMoveUpdatedComponent(Delta, Rotation, true, Hit);

		if (Hit.IsValidBlockingHit())
		{
			HandleImpact(Hit, DeltaTime, Delta);
			// Try to slide the remaining distance along the surface.
			SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);
		}
	}

	UpdateComponentVelocity();
}

void UGCBasePawnMovementComponent::JumpStart()
{
	VerticalVelocity = InitialJumpVelocity * FVector::UpVector;
}

bool UGCBasePawnMovementComponent::IsFalling() const
{
	return bIsFalling;
}
