// Fill out your copyright notice in the Description page of Project Settings.
//#define QUICKHULL_IMPLEMENTATION

#include "PCGActor.h"
#include "PCG.h"


// Sets default values
APCGActor::APCGActor()
{
	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	RootComponent = mesh;

	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialFinder(TEXT("Material'/Game/M_PCG.M_PCG'"));

	if (MaterialFinder.Succeeded())
	{
		material = MaterialFinder.Object;

		mesh->SetMaterial(0, material);
	}
}

// Called when the game starts or when spawned
void APCGActor::BeginPlay()
{
	Super::BeginPlay();
	
	this->SetActorLocation(FVector::ZeroVector);
	this->SetActorRotation(FQuat::Identity);

	float radius = 100.0f;

	GeneratePoints(points, radius, 2000);

	FlushPersistentDebugLines(GetWorld());

	for (auto &point : points)
	{
		DrawDebugPoint(GetWorld(), point, 4.0f, FColor::Red, true, -1.0F, (uint8)'\000');
	}

	CalculateMesh(points, meshpoints, meshindexes, meshnormals, faces, meshfaces);

	CalculateMorphMesh(meshpoints, meshindexes, spheremeshpoints, spheremeshnormals, radius);
}

// Called every frame
void APCGActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APCGActor::GeneratePoints(TArray<FVector> &points, const float &radius, const int &count)
{
	FVector v;
	
	for (int i = 0; i < count; ++i)
	{
		/*float a0 = FMath::FRandRange(0.0f, 2 * PI);
		float a1 = FMath::FRandRange(0.0f, 2 * PI);
		v.Z = FMath::Sin(a0) * radius;
		v.X = FMath::Cos(a1) * FMath::Cos(a0) * FMath::FRandRange(0.0f, radius);
		v.Y = FMath::Sin(a1) * FMath::Cos(a0) * FMath::FRandRange(0.0f, radius);*/

		v.X = FMath::FRandRange(-radius, radius);
		v.Y = FMath::FRandRange(-radius, radius);
		v.Z = FMath::FRandRange(-radius, radius);

		if (v != FVector::ZeroVector
			&& v.X <= radius && v.X >= -radius
			&& v.Y <= radius && v.Y >= -radius
			&& v.Z <= radius && v.Z >= -radius)
			points.AddUnique(v);
	};
}

void APCGActor::CalculateMesh(const TArray<FVector> &points, TArray<FVector> &meshpoints, TArray<int32> &meshindexes, TArray<FVector> &meshnormals, TArray<FQHFace> &faces, TArray<FQHFace> &meshfaces)
{
	FVector xMin, xMax, yMin, yMax, zMin, zMax;

	for (auto &point : points)
	{
		if (point.X < xMin.X)
		{
			xMin = point;
		}
		else if (point.X > xMax.X)
		{
			xMax = point;
		}

		if (point.Y < yMin.Y)
		{
			yMin = point;
		}
		else if (point.Y > yMax.Y)
		{
			yMax = point;
		}

		if (point.Z < zMin.Z)
		{
			zMin = point;
		}
		else if (point.Z > zMax.Z)
		{
			zMax = point;
		}
	};

	TArray<FVector> EP6;

	EP6.Add(xMin);
	EP6.Add(xMax);
	EP6.Add(yMin);
	EP6.Add(yMax);
	EP6.Add(zMin);
	EP6.Add(zMax);

	FVector v0, v1, v2, v3;
	float dist = 0.0f;
	float dist_l = 0.0f;
	float dist_p = 0.0f;

	for (auto &point0 : EP6)
	{
		for (auto &point1 : EP6)
		{
			if (point0 == point1)
				continue;

			if (dist < FVector::Dist(point0, point1))
			{
				dist = FVector::Dist(point0, point1);
				v0 = point0;
				v1 = point1;
			};
		};
	};

	for (auto &point : EP6)
	{
		float d = FMath::PointDistToLine(point, v0, v1);

		if (d > dist_l)
		{
			dist_l = d;
			v2 = point;
		};
	}

	FVector centroid = FVector((v0.X + v1.X + v2.X) / 3, (v0.Y + v1.Y + v2.Y) / 3, (v0.Z + v1.Z + v2.Z) / 3);

	FVector normal = FVector::CrossProduct((v1 - v0), (v2 - v0));

	normal.Normalize();

	for (auto &point : points)
	{
		float d = FVector::PointPlaneDist(point, centroid, normal);

		if (d > dist_p)
		{
			dist_p = d;
			v3 = point;
		};
	}

	if (FVector::DotProduct((v3 - centroid), normal) > 0)
	{
		normal = -normal;
	};

	FQHFace tetrahedron[4];

	tetrahedron[0] = FQHFace(v0, v1, v2, v3);
	tetrahedron[1] = FQHFace(v1, v2, v3, v0);
	tetrahedron[2] = FQHFace(v2, v3, v0, v1);
	tetrahedron[3] = FQHFace(v3, v0, v1, v2);

	for (auto &face : tetrahedron)
	{
		QHStep(face, points, faces);
	}

	for (auto &face : faces)
	{
		meshpoints.AddUnique(face.a);
		meshpoints.AddUnique(face.b);
		meshpoints.AddUnique(face.c);
	}

	for (auto &face : faces)
	{
		if (face.a == face.b || face.a == face.c || face.b == face.c)
			continue;

		meshfaces.AddUnique(face);
	}

	for (auto &meshface : meshfaces)
	{
		int32 index_a, index_b, index_c;

		for (int32 i = 0; i < meshpoints.Num(); i++)
		{
			if (meshface.a == meshpoints[i])
			{
				index_a = i;
			}

			if (meshface.b == meshpoints[i])
			{
				index_b = i;
			}

			if (meshface.c == meshpoints[i])
			{
				index_c = i;
			}
		}

		meshindexes.Add(index_a);
		meshindexes.Add(index_b);
		meshindexes.Add(index_c);
	}

	for (auto &meshface : meshfaces)
	{
		FVector n_a = meshface.a;
		FVector n_b = meshface.b;
		FVector n_c = meshface.c;

		n_a.Normalize();
		n_b.Normalize();
		n_c.Normalize();

		meshnormals.Add(n_a);
		meshnormals.Add(n_b);
		meshnormals.Add(n_c);
	}
}

void APCGActor::CalculateMorphMesh(const TArray<FVector> &meshpoints, const TArray<int32> &meshindexes, TArray<FVector> &spheremeshpoints, TArray<FVector> & spheremeshnormals, const float &radius)
{
	for (auto &meshpoint : meshpoints)
	{
		FVector spheremeshpoint = meshpoint;
		spheremeshpoint.Normalize();
		spheremeshpoint = radius * spheremeshpoint;
		spheremeshpoints.Add(spheremeshpoint);
	}

	for (auto &meshindex : meshindexes)
	{
		FVector spheremeshnormal = meshpoints[meshindex];
		spheremeshnormal.Normalize();
		spheremeshnormals.Add(spheremeshnormal);
	}
}

void APCGActor::DrawDebugQHFace(const FQHFace &face, const FColor &color)
{
	DrawDebugLine(GetWorld(), face.a, face.b, color, true, -1.0F, (uint8)'\000', 1);
	DrawDebugLine(GetWorld(), face.b, face.c, color, true, -1.0F, (uint8)'\000', 1);
	DrawDebugLine(GetWorld(), face.a, face.c, color, true, -1.0F, (uint8)'\000', 1);

	//DrawDebugLine(GetWorld(), face.centroid, face.centroid + face.normal * 10, FColor::Yellow, true, -1.0F, (uint8)'\000', 1);

	DrawDebugMesh(GetWorld(), { face.a, face.b, face.c }, { 0, 1, 2 }, FColor::Blue, true, -1.0f, (uint8)'\000');
}

void APCGActor::QHStep(const FQHFace &face, const TArray<FVector> &points, TArray<FQHFace> &faces)
{
	TArray<FVector> facepoints;

	for (auto &point : points)
	{
		if (FVector::DotProduct((point - face.centroid), face.normal) > 0)
		{
			facepoints.AddUnique(point);
		};
	}

	if (facepoints.Num() > 0)
	{
		FVector face_point_max;
		float dist_face_point_max = 0.0f;

		for (auto &facepoint : facepoints)
		{
			float d = FVector::PointPlaneDist(facepoint, face.centroid, face.normal);

			if (d > dist_face_point_max)
			{
				dist_face_point_max = d;
				face_point_max = facepoint;
			};
		}

		FQHFace newfaces[3];

		newfaces[0] = FQHFace(face_point_max, face.a, face.b, face.c);
		newfaces[1] = FQHFace(face_point_max, face.b, face.c, face.a);
		newfaces[2] = FQHFace(face_point_max, face.c, face.a, face.b);

		for (auto &newface : newfaces)
		{
			QHStep(newface, facepoints, faces);
		}
	}
	else
	{
		faces.Add(face);
	}
}

void APCGActor::Step0()
{
	FlushPersistentDebugLines(GetWorld());

	mesh->ClearAllMeshSections();
	mesh->CreateMeshSection_LinearColor(0, meshpoints, meshindexes, meshnormals, {}, {}, {}, false);
}

void APCGActor::Step1()
{
	FlushPersistentDebugLines(GetWorld());

	mesh->ClearAllMeshSections();
	mesh->CreateMeshSection_LinearColor(0, spheremeshpoints, meshindexes, spheremeshnormals, {}, {}, {}, false);
}

void APCGActor::Step2()
{
	float radius = 100.0f;

	points.Empty();
	faces.Empty();
	meshfaces.Empty();
	meshpoints.Empty();
	meshindexes.Empty();
	meshnormals.Empty();
	spheremeshpoints.Empty();
	spheremeshnormals.Empty();

	GeneratePoints(points, radius, 2000);

	mesh->ClearAllMeshSections();

	FlushPersistentDebugLines(GetWorld());

	for (auto &point : points)
	{
		DrawDebugPoint(GetWorld(), point, 4.0f, FColor::Red, true, -1.0F, (uint8)'\000');
	}

	CalculateMesh(points, meshpoints, meshindexes, meshnormals, faces, meshfaces);

	CalculateMorphMesh(meshpoints, meshindexes, spheremeshpoints, spheremeshnormals, radius);
}