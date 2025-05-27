#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define N 30                // N
#define READ_LEN 12         // read의 길이(L)
#define READS_COUNT 3       // read 개수 (M)
#define MAX_MISMATCH 2      // 허용 가능한 최대 mismatch 수
#define PARTS 3             // read를 나눌 부분 수 (d+1)
#define PARTLEN 4           // read를 d+1로 나눈 파트 길이 (L / PARTS)
#define q 10007             // 해시 계산에 사용할 소수 q

// 원본 reference
char reference[N + 1] = "ACGTTGCAAGTCGATCGTACGATCGTACGT";

// read
char* reads[READS_COUNT] = {
    "GTCGTTCGTCCG",
    "ACGCTGCATGTC",
    "CCGATCGTACGT"
};

// 해시 테이블(체이닝 방식 사용)
typedef struct HashEntry {
    int pos;                   // reference에서의 시작 위치
    struct HashEntry* next;    // 충돌 시 연결 리스트로 연결
} HashEntry;

#define HASH_SIZE 10007       // 해시 테이블 크기
HashEntry* hashTable[HASH_SIZE]; // 해시 테이블 전역 배열

// 문자-> 숫자
int hash_data(char c) {
    switch (c) {
    case 'A': return 0;
    case 'C': return 1;
    case 'G': return 2;
    case 'T': return 3;
    }
    return -1;
}

// 라빈카프 해시 계산 함수 (4진수 기반)
unsigned int computeHash(char* str, int len) {
    unsigned int hash = 0;
    for (int i = 0; i < len; i++) {
        hash = (hash * 4 + hash_data(str[i])) % q;
    }
    return hash;
}

// 해시 테이블에 위치 정보 삽입
void insertToHash(unsigned int hash, int pos) {
    HashEntry* entry = (HashEntry*)malloc(sizeof(HashEntry));
    entry->pos = pos;
    entry->next = hashTable[hash];
    hashTable[hash] = entry;
}

// reference를 기반으로 해시 테이블 구성
void buildHashTable(int k) {
    int refLen = strlen(reference);
    if (refLen < k) return;

    // 해시 계산을 위한 초기값 및 최고차 항 계산
    unsigned int hash = 0;
    unsigned int highest_power = 1;
    for (int i = 0; i < k - 1; i++) highest_power = (highest_power * 4) % q;

    // 첫 서브스트링 해시 삽입
    for (int i = 0; i < k; i++) {
        hash = (hash * 4 + hash_data(reference[i])) % q;
    }
    insertToHash(hash, 0);

    // 이후 rolling hash로 나머지 삽입
    for (int i = 1; i <= refLen - k; i++) {
        hash = (hash + q - (hash_data(reference[i - 1]) * highest_power) % q) % q; //앞문자 삭제 (4)231
        hash = (hash * 4 + hash_data(reference[i + k - 1])) % q; //새로운 뒷문자 추가 (4)2317 -> 새로운해시
        insertToHash(hash, i);
    }
}

// 두 문자열을 비교하여 mismatch 개수 반환 (허용 초과시 -1 반환)
int compareWithMismatch(char* a, char* b, int len) {
    int mismatches = 0;
    for (int i = 0; i < len; i++) {
        if (a[i] != b[i]) {
            mismatches++;
            if (mismatches > MAX_MISMATCH)
                return -1;
        }
    }
    return mismatches;
}

int main() {
    // reference 복사본을 result에 저장 (돌연변이 결과 출력용)
    char result[sizeof(reference)];
    strcpy(result, reference); //원본에서 d만 바꾸는 식으로 진행

    // 해시 테이블 구성
    buildHashTable(PARTLEN);

    // 각 read에 대해 처리
    for (int r = 0; r < READS_COUNT; r++) {
        char* read = reads[r];
        int matched = 0; // 현재 리드가 reference에 매치되었는지 여부

        // read를 PARTS(3)로 나누어 하나씩 해시 비교
        for (int part = 0; part < PARTS; part++) {
            char sub[5]; // part 길이 + 1
            strncpy(sub, &read[part * PARTLEN], PARTLEN);
            sub[PARTLEN] = '\0';

            // 해시값 계산 후 해시 테이블 검색
            unsigned int h = computeHash(sub, PARTLEN); //해시계산
            HashEntry* entry = hashTable[h]; //검색

            while (entry) {
                // 리드를 원래 위치로 복원하기 위해 보정
                int ref_start = entry->pos - part * PARTLEN;

                // 범위 벗어나지 않도록 확인
                if (ref_start >= 0 && ref_start + READ_LEN <= strlen(reference)) {
                    char* refSegment = &reference[ref_start];

                    int mismatch = compareWithMismatch(read, refSegment, READ_LEN); //mismatch 횟수 리턴

                    if (mismatch >= 0) {
                        // mismatch 허용 범위(d) 이내 → 돌연변이 수정
                        for (int i = 0; i < READ_LEN; i++) {
                            if (read[i] != refSegment[i]) {
                                result[ref_start + i] = read[i]; // mismatch 수정
                            }
                        }
                        matched = 1; //현재 리드가 reference에 매치o
                        break;
                    }
                }
                entry = entry->next;
            }

            if (matched) break; // 한 파트에서 성공하면(matched = 1) 다음 read로 넘어감
        }
    }

    // 결과 출력
    printf("원본:\n%s\n", reference);
    printf("결과:\n%s\n", result);

    return 0;
}
