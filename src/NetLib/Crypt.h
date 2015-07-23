#pragma once

class CCrypt
{
public:
	static BOOL Encrypt(BYTE *source, BYTE *destination, DWORD length);
	static BOOL Decrypt(BYTE *source, BYTE *destination, DWORD length);
};



// <암호화 1>
// RSA 키 랜덤 생성
// 접속 완료시 클라이언트의 퍼블릭키를 보낸다.(AcceptEx 설정시 값을 받도록 한다!!)
// 서버의 퍼블릭 키를 받는다.
// 이제부터는 서버의 퍼블릭 키로 암호화 하여 전송
// 서버로 부터 받은 데이터는 클라의 프라이빗키로 복화화 하여 해독
// (암호화 키를 키테이블 시드로 사용해도 된다.)

// <암호화 2>
// SEED를 통해 Encode, Decode 테이블을 생성(random 클래스 필요)
// 특정 기준에 다라 Encode 테이블의 데이터로 xor 연산

// Checksum 계산
// PacketSendCount를 활용해 복잡도 증가
// 소켓 ID를 활용해 복잡도 증가