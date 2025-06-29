#include "tLib.h"

void tNodeInit(tNode* node)
{
    node->nextNode = node;
    node->preNode = node;
}

#define firstNode  headNode.nextNode
#define lastNode   headNode.preNode

/* 链表初始化 */
void tListInit(tList* list)
{
    list->firstNode = &(list->headNode);
    list->lastNode = &(list->headNode);
    list->nodeCount = 0;
}

/* 接口:链表结点数量 */
uint32_t tListCount(tList* list)
{
    return list->nodeCount;
}

/* 接口:链表首个结点 */
tNode* tListFirst(tList* list)
{
    tNode* node = (tNode*)0;
    if(list->nodeCount != 0)
    {
        node = list->firstNode;
    }
    return node;
}

/* 接口:链表最后一个结点 */
tNode*tListLast(tList* list)
{
    tNode* node = (tNode*)0;

    if(list->nodeCount != 0)
    {
        node = list->lastNode;//这个链表是首尾闭环的,所以访问头结点前一个就可以找到尾结点
    }
    return node;
}

/* 返回链表中指定结点的前一个结点 */
tNode* tListPpre(tList* list,  tNode* node)
{
    if(node->preNode == node)//前一个结点就是自己,说明处于空链表中
    {
        return (tNode*)0;
    }
    else
    {
        return node->preNode;
    }
}

/* 返回链表中指定结点的后一个节点 */
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

/* 清除所有结点 */
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

/* 将指定结点添加到链表头部 头结点指list父结点的下一个*/
void tListAddFirst(tList* list, tNode* node)
{
    node->preNode = list->firstNode->preNode;//
    node->nextNode =  list->firstNode;

    list->firstNode->preNode = node;
    list->firstNode = node;
    list->nodeCount++;
}

/* 尾部插入结点 */
void tListAddLast(tList* list, tNode* node)
{
    node->nextNode = &(list->headNode);//尾结点指回父结点
    node->preNode = list->lastNode;//头结点的前一个是尾节点

    list->lastNode->nextNode = node;
    list->lastNode = node;
    list->nodeCount++;
}

/* 移除链表中的第1个结点 */
tNode* tListRemoveFirst(tList* list)
{
    tNode* node = (tNode*)0;

    if(list->nodeCount != 0)
    {
        node = list->firstNode;//头结点赋予
        node->nextNode->preNode = &(list->headNode);//头结点的下一个结点指向父结点
        list->firstNode = node->nextNode;//头结点的下一个指向原结点下下个结点
        list->nodeCount--;
    }
    return node;
}

/* 指定结点插到某个结点后面 */
void tListInsertAfter(tList* list, tNode* nodeAfter, tNode* nodeToInsert)
{
    nodeToInsert->preNode = nodeAfter;
    nodeToInsert->nextNode = nodeAfter->nextNode;

    nodeAfter->nextNode->preNode = nodeToInsert;
    nodeAfter->nextNode = nodeToInsert;

    list->nodeCount++;
}

/* 将指定结点从链表中移除 */
void tListRemove(tList* list, tNode* node)
{
    node->preNode->nextNode = node->nextNode;
    node->nextNode->preNode = node->preNode;
    list->nodeCount--;
}
