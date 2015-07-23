#pragma once

class CRandomGen
{
public:
	CRandomGen(VOID);
	virtual ~CRandomGen(VOID);

private:
	UINT	mSeed;

public:
	BOOL	Init(UINT seed);

	INT		Rand(VOID);
	INT		SimpleRandom(VOID);
	INT		ComplexRandom(VOID);
};