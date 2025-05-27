#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define N 30                // N
#define READ_LEN 12         // read�� ����(L)
#define READS_COUNT 3       // read ���� (M)
#define MAX_MISMATCH 2      // ��� ������ �ִ� mismatch ��
#define PARTS 3             // read�� ���� �κ� �� (d+1)
#define PARTLEN 4           // read�� d+1�� ���� ��Ʈ ���� (L / PARTS)
#define q 10007             // �ؽ� ��꿡 ����� �Ҽ� q

// ���� reference ����
char reference[N + 1] = "ACGTTGCAAGTCGATCGTACGATCGTACGT";

// �� ��� read��
char* reads[READS_COUNT] = {
    "GTCGTTCGTCCG",
    "ACGCTGCATGTC",
    "CCGATCGTACGT"
};

// �ؽ� ���̺� ����� ����ü ���� (ü�̴� ��� ���)
typedef struct HashEntry {
    int pos;                   // reference������ ���� ��ġ
    struct HashEntry* next;    // �浹 �� ���� ����Ʈ�� ����
} HashEntry;

#define HASH_SIZE 10007       // �ؽ� ���̺� ũ��
HashEntry* hashTable[HASH_SIZE]; // �ؽ� ���̺� ���� �迭

// ���ڸ� ���ڷ� ���� (A=0, C=1, G=2, T=3)
int hash_data(char c) {
    switch (c) {
    case 'A': return 0;
    case 'C': return 1;
    case 'G': return 2;
    case 'T': return 3;
    }
    return -1;
}

// ���-ī�� �ؽ� ��� �Լ� (4���� ���)
unsigned int computeHash(char* str, int len) {
    unsigned int hash = 0;
    for (int i = 0; i < len; i++) {
        hash = (hash * 4 + hash_data(str[i])) % q;
    }
    return hash;
}

// �ؽ� ���̺� ��ġ ���� ����
void insertToHash(unsigned int hash, int pos) {
    HashEntry* entry = (HashEntry*)malloc(sizeof(HashEntry));
    entry->pos = pos;
    entry->next = hashTable[hash];
    hashTable[hash] = entry;
}

// reference�� ������� �ؽ� ���̺� ����
void buildHashTable(int k) {
    int refLen = strlen(reference);
    if (refLen < k) return;

    // �ؽ� ����� ���� �ʱⰪ �� �ְ��� �� ���
    unsigned int hash = 0;
    unsigned int highest_power = 1;
    for (int i = 0; i < k - 1; i++) highest_power = (highest_power * 4) % q;

    // ù ���꽺Ʈ�� �ؽ� ����
    for (int i = 0; i < k; i++) {
        hash = (hash * 4 + hash_data(reference[i])) % q;
    }
    insertToHash(hash, 0);

    // ���� rolling hash�� ������ ����
    for (int i = 1; i <= refLen - k; i++) {
        hash = (hash + q - (hash_data(reference[i - 1]) * highest_power) % q) % q; //�չ��� ���� (4)231
        hash = (hash * 4 + hash_data(reference[i + k - 1])) % q; //���ο� �޹��� �߰� (4)2317 -> ���ο��ؽ�
        insertToHash(hash, i);
    }
}

// �� ���ڿ��� ���Ͽ� mismatch ���� ��ȯ (��� �ʰ��� -1 ��ȯ)
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
    // reference ���纻�� result�� ���� (�������� ��� ��¿�)
    char result[sizeof(reference)];
    strcpy(result, reference); //�������� d�� �ٲٴ� ������ ����

    // �ؽ� ���̺� ����
    buildHashTable(PARTLEN);

    // �� read�� ���� ó��
    for (int r = 0; r < READS_COUNT; r++) {
        char* read = reads[r];
        int matched = 0; // ���� ���尡 reference�� ��ġ�Ǿ����� ����

        // read�� PARTS(3)�� ������ �ϳ��� �ؽ� ��
        for (int part = 0; part < PARTS; part++) {
            char sub[5]; // part ���� + 1
            strncpy(sub, &read[part * PARTLEN], PARTLEN);
            sub[PARTLEN] = '\0';

            // �ؽð� ��� �� �ؽ� ���̺� �˻�
            unsigned int h = computeHash(sub, PARTLEN); //�ؽð��
            HashEntry* entry = hashTable[h]; //�˻�

            while (entry) {
                // ���带 ���� ��ġ�� �����ϱ� ���� ����
                int ref_start = entry->pos - part * PARTLEN;

                // ���� ����� �ʵ��� Ȯ��
                if (ref_start >= 0 && ref_start + READ_LEN <= strlen(reference)) {
                    char* refSegment = &reference[ref_start];

                    int mismatch = compareWithMismatch(read, refSegment, READ_LEN); //mismatch Ƚ�� ����

                    if (mismatch >= 0) {
                        // mismatch ��� ����(d) �̳� �� �������� ����
                        for (int i = 0; i < READ_LEN; i++) {
                            if (read[i] != refSegment[i]) {
                                result[ref_start + i] = read[i]; // mismatch ����
                            }
                        }
                        matched = 1; //���� ���尡 reference�� ��ġo
                        break;
                    }
                }
                entry = entry->next;
            }

            if (matched) break; // �� ��Ʈ���� �����ϸ�(matched = 1) ���� read�� �Ѿ
        }
    }

    // ��� ���
    printf("����:\n%s\n", reference);
    printf("���:\n%s\n", result);

    return 0;
}
