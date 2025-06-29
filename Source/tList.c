#include "tLib.h"

void tNodeInit(tNode* node)
{
    node->nextNode = node;
    node->preNode = node;
}

#define firstNode  headNode.nextNode
#define lastNode   headNode.preNode

/* �����ʼ�� */
void tListInit(tList* list)
{
    list->firstNode = &(list->headNode);
    list->lastNode = &(list->headNode);
    list->nodeCount = 0;
}

/* �ӿ�:���������� */
uint32_t tListCount(tList* list)
{
    return list->nodeCount;
}

/* �ӿ�:�����׸���� */
tNode* tListFirst(tList* list)
{
    tNode* node = (tNode*)0;
    if(list->nodeCount != 0)
    {
        node = list->firstNode;
    }
    return node;
}

/* �ӿ�:�������һ����� */
tNode*tListLast(tList* list)
{
    tNode* node = (tNode*)0;

    if(list->nodeCount != 0)
    {
        node = list->lastNode;//�����������β�ջ���,���Է���ͷ���ǰһ���Ϳ����ҵ�β���
    }
    return node;
}

/* ����������ָ������ǰһ����� */
tNode* tListPpre(tList* list,  tNode* node)
{
    if(node->preNode == node)//ǰһ���������Լ�,˵�����ڿ�������
    {
        return (tNode*)0;
    }
    else
    {
        return node->preNode;
    }
}

/* ����������ָ�����ĺ�һ���ڵ� */
tNode* tListNext(tList* list, tNode* node)
{
    if(node->nextNode == node)
    {
        return (tNode*)0;
    }
    else
    {
        return node->nextNode;
    }
} 

/* ������н�� */
void tListRemoveAll(tList* list)
{
    uint32_t count;
    tNode* nextNode;
    nextNode = list->firstNode;
    for(count = list->nodeCount; count != 0; count--)
    {
        tNode* currentNode = nextNode;
        nextNode = nextNode->nextNode;

        currentNode->nextNode = currentNode;
        currentNode->preNode = currentNode;
    }
    list->firstNode = &(list->headNode);
    list->lastNode = &(list->headNode);
    list->nodeCount = 0;
}

/* ��ָ�������ӵ�����ͷ�� ͷ���ָlist��������һ��*/
void tListAddFirst(tList* list, tNode* node)
{
    node->preNode = list->firstNode->preNode;//
    node->nextNode =  list->firstNode;

    list->firstNode->preNode = node;
    list->firstNode = node;
    list->nodeCount++;
}

/* β�������� */
void tListAddLast(tList* list, tNode* node)
{
    node->nextNode = &(list->headNode);//β���ָ�ظ����
    node->preNode = list->lastNode;//ͷ����ǰһ����β�ڵ�

    list->lastNode->nextNode = node;
    list->lastNode = node;
    list->nodeCount++;
}

/* �Ƴ������еĵ�1����� */
tNode* tListRemoveFirst(tList* list)
{
    tNode* node = (tNode*)0;

    if(list->nodeCount != 0)
    {
        node = list->firstNode;//ͷ��㸳��
        node->nextNode->preNode = &(list->headNode);//ͷ������һ�����ָ�򸸽��
        list->firstNode = node->nextNode;//ͷ������һ��ָ��ԭ������¸����
        list->nodeCount--;
    }
    return node;
}

/* ָ�����嵽ĳ�������� */
void tListInsertAfter(tList* list, tNode* nodeAfter, tNode* nodeToInsert)
{
    nodeToInsert->preNode = nodeAfter;
    nodeToInsert->nextNode = nodeAfter->nextNode;

    nodeAfter->nextNode->preNode = nodeToInsert;
    nodeAfter->nextNode = nodeToInsert;

    list->nodeCount++;
}

/* ��ָ�������������Ƴ� */
void tListRemove(tList* list, tNode* node)
{
    node->preNode->nextNode = node->nextNode;
    node->nextNode->preNode = node->preNode;
    list->nodeCount--;
}
