#pragma once

class CStage;
class CStageManager
{
private:
	int			m_iStage;
	int			m_iNextStage;
	CStage*		m_pStage;

public:
	bool Init(int iStage);
	int UpdateFrame();
	int Render();

private:
	bool ChangeStage(int iStage, bool bInit = false);

public:
	CStageManager();
	~CStageManager();
};

