// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "PCGActor.generated.h"

USTRUCT()
struct FQHFace
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(VisibleAnywhere)
	FVector a;

	UPROPERTY(VisibleAnywhere)
	FVector b;

	UPROPERTY(VisibleAnywhere)
	FVector c;

	UPROPERTY(VisibleAnywhere)
	FVector normal;

	UPROPERTY(VisibleAnywhere)
	FVector centroid;

	UPROPERTY(VisibleAnywhere)
	FVector point4;

	FQHFace() {};

	FQHFace(FVector a, FVector b, FVector c, FVector d)
	{
		this->a = a;
		this->b = b;
		this->c = c;
		this->point4 = d;

		this->centroid = FVector((this->a.X + this->b.X + this->c.X) / 3, (this->a.Y + this->b.Y + this->c.Y) / 3, (this->a.Z + this->b.Z + this->c.Z) / 3);
		
		this->normal = FVector::CrossProduct((this->b - this->a), (this->c - this->a));

		this->normal.Normalize();

		if (FVector::DotProduct((this->point4 - this->centroid), this->normal) > 0)
		{
			this->normal = -this->normal;
		};
	};

	bool operator==(const FQHFace& F) const
	{
		return a == F.a && b == F.b && c == F.c;
	}
};

UCLASS()
class PCG_API APCGActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APCGActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	TArray<FVector> points;

	UPROPERTY(VisibleAnywhere)
	TArray<FQHFace> faces;

	UPROPERTY(VisibleAnywhere)
	TArray<FQHFace> meshfaces;

	UPROPERTY(VisibleAnywhere)
	TArray<FVector> meshpoints;

	UPROPERTY(VisibleAnywhere)
	TArray<int32> meshindexes;

	UPROPERTY(VisibleAnywhere)
	TArray<FVector> meshnormals;

	UPROPERTY(VisibleAnywhere)
	TArray<FVector> spheremeshpoints;

	UPROPERTY(VisibleAnywhere)
	TArray<FVector> spheremeshnormals;

	UPROPERTY(VisibleAnywhere)
	UMaterial* material;

	UFUNCTION(BlueprintCallable, Category = "PCG", meta = (DisplayName = "Step0"))
	void Step0();

	UFUNCTION(BlueprintCallable, Category = "PCG", meta = (DisplayName = "Step1"))
	void Step1();

	UFUNCTION(BlueprintCallable, Category = "PCG", meta = (DisplayName = "Step2"))
	void Step2();

	void GeneratePoints(TArray<FVector> &points, const float &radius, const int &count);

	void CalculateMesh(const TArray<FVector> &poins, TArray<FVector> &meshpoints, TArray<int32> &meshindexes, TArray<FVector> &meshnormals, TArray<FQHFace> &faces, TArray<FQHFace> &meshfaces);

	void CalculateMorphMesh(const TArray<FVector> &meshpoints, const TArray<int32> &meshindexes, TArray<FVector> &spheremeshpoints, TArray<FVector> & spheremeshnormals, const float &radius);

	void DrawDebugQHFace(const FQHFace &face, const FColor &color);

	void QHStep(const FQHFace &face, const TArray<FVector> &points, TArray<FQHFace> &faces);

private:
	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent * mesh;
	
	int si;
};
