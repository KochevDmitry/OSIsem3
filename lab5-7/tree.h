#include <stdio.h>


typedef struct TNode {
    int data;
    struct TNode *firstChild; // указатель на первого потомка
    struct TNode *nextBrother; // указатель на следующего брата
} TNode;


TNode *createNode(int data) {
    TNode *newNode = (TNode *)malloc(sizeof(TNode));
    if (newNode != NULL) {
        newNode->data = data;
        newNode->firstChild = NULL;
        newNode->nextBrother = NULL;
    }
    return newNode;
}


void addChild(TNode *parent, int data) {
    TNode *newChild = createNode(data);
    if (newChild == NULL) {
        fprintf(stderr, "Failed to create a new child node.\n");
        return;
    }
    // Если у узла нет потомков, добавляем нового потомка
    if (parent->firstChild == NULL) {
        parent->firstChild = newChild;
    } else {
        // Если у узла уже есть потомок, идем по списку братьев
        TNode *brother = parent->firstChild;
        while (brother->nextBrother != NULL) {
            brother = brother->nextBrother;
        }
        brother->nextBrother = newChild;
    }
}
//  TreeNode *root = createNode(1);
//  addChild(root, 2);


int findPath(TNode *currentNode, int targetData, int *path, int depth) {
    if (currentNode == NULL) {
        return 0;
    }
    // Если текущий узел содержит искомые данные, завершаем рекурсию
    if (currentNode->data == targetData) {
        path[depth] = -1;  // 0 означает, что текущий узел содержит искомые данные
        return 1;
    }
    // Пытаемся найти путь в потомках
    int childIndex = 1;
    if (findPath(currentNode->firstChild, targetData, path, depth + 1)) {
        path[depth] = childIndex;
        return 1;
    }
    // Пытаемся найти путь в братьях
    if (findPath(currentNode->nextBrother, targetData, path, depth)) {
        path[depth] = childIndex;
        return 1;
    }
    return 0;  // Искомый узел не найден
}

// int targetData = 6;
// int path[100];  // здесь будет сохранен путь
// int found = findPath(root, targetData, path, 0);


// Функция для удаления узла и всех его потомков
void deleteNodeAndChildren(TNode *parent, TNode *toDelete) {
    if (parent == NULL || toDelete == NULL) {
        return;
    }

    // Если удаляемый узел - первый потомок родителя
    if (parent->firstChild == toDelete) {
        parent->firstChild = toDelete->nextBrother;
    } else {
        // Если удаляемый узел - брат
        TNode *brother = parent->firstChild;
        while (brother != NULL && brother->nextBrother != toDelete) {
            brother = brother->nextBrother;
        }

        if (brother != NULL) {
            brother->nextBrother = toDelete->nextBrother;
        }
    }

    // Обновляем указатель на следующего брата у левого брата удаляемого узла
    if (toDelete->nextBrother != NULL && parent->firstChild != toDelete) {
        TNode *leftBrother = parent->firstChild;
        while (leftBrother->nextBrother != toDelete) {
            leftBrother = leftBrother->nextBrother;
        }

        leftBrother->nextBrother = toDelete->nextBrother;
    }

    // Рекурсивно удаляем всех потомков узла
    TNode *child = toDelete->firstChild;
    while (child != NULL) {
        TNode *nextChild = child->nextBrother;
        deleteNodeAndChildren(toDelete, child);
        child = nextChild;
    }

    free(toDelete);
}


// Функция для поиска узла по значению
TNode *findNodeAndParent(TNode *root, int targetData, TNode **parent) {
    if (root == NULL) {
        return NULL;
    }

    if (root->data == targetData) {
        return root;
    }

    // Рекурсивный вызов для потомков
    TNode *result = findNodeAndParent(root->firstChild, targetData, &root);
    if (result != NULL) {
        *parent = root;
        return result;
    }

    // Рекурсивный вызов для братьев
    return findNodeAndParent(root->nextBrother, targetData, parent);
}
// int targetData = 4;
// TreeNode *parent = NULL;
// TreeNode *toFind = findNodeAndParent(root, targetData, &parent);

TNode* find_node(TNode *root, int data) {
    if (root == NULL) {
        return NULL; // Дошли до конца поддерева, узел не найден
    }

    if (root->data == data) {
        return root; // Узел найден
    }

    // Рекурсивно ищем в потомках и братьях
    TNode *found_in_child = find_node(root->firstChild, data);
    if (found_in_child != NULL) {
        return found_in_child; // Найден в потомках
    }

    return find_node(root->nextBrother, data); // Ищем в братьях
}


void freeTree(TNode *root) {
    if (root == NULL) {
        return; // Базовый случай: пустое поддерево
    }

    // Рекурсивный вызов для всех потомков текущего узла
    freeTree(root->firstChild);

    // Рекурсивный вызов для всех братьев текущего узла
    freeTree(root->nextBrother);

    // Освобождение памяти для текущего узла
    free(root);
}


void printTree(TNode* root, int depth) { // глубина всегда в начале 0
    if (root == NULL) {
        return;
    }
    // Вывод текущего узла с отступом, зависящим от глубины в дереве
    for (int i = 0; i < depth; ++i) {
        printf("  ");
    }
    printf("%d\n", root->data);

    // Рекурсивный вызов для всех потомков текущего узла
    printTree(root->firstChild, depth + 1);

    // Рекурсивный вызов для всех братьев текущего узла
    printTree(root->nextBrother, depth);
}