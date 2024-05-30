#include <iostream>
#ifndef BINARY_TREE_H
#define BINARY_TREE_H

class BinarySearchTree {
public:
    BinarySearchTree();
    ~BinarySearchTree();
    void insert(int x);
    void remove(int x);
    void display();
    void search(int x);

private:
    struct Node;
    Node* root;
    Node* clear(Node* currentNode);
    Node* insert(int valueToInsert, Node* currentNode);
    Node* remove(int valueToRemove, Node* currentNode);
    Node* findMinimal(Node* currentNode);
    Node* findMaximum(Node* currentNode);
    Node* find(Node* currentNode, int valueToFind);
    void inorder(Node* currentNode);
};

#endif  // !BINARY_TREE_H

struct BinarySearchTree::Node {
    int data;
    BinarySearchTree::Node* left;
    BinarySearchTree::Node* right;
};

BinarySearchTree::Node* BinarySearchTree::clear(Node* currentNode) {
    if (currentNode != nullptr) {
        clear(currentNode->left);
        clear(currentNode->right);
        delete currentNode;
    }
    return nullptr;
}

BinarySearchTree::Node* BinarySearchTree::insert(int valueToInsert, Node* currentNode) {
    if (currentNode == nullptr) {
        currentNode = new Node;
        currentNode->data = valueToInsert;
        currentNode->left = currentNode->right = nullptr;

    } else if (valueToInsert < currentNode->data) {
        currentNode->left = insert(valueToInsert, currentNode->left);
    } else if (valueToInsert > currentNode->data) {
        currentNode->right = insert(valueToInsert, currentNode->right);
    }
    return currentNode;
}



BinarySearchTree::Node* BinarySearchTree::remove(int valueToRemove, Node* currentNode) {
    Node* temp;
    if (currentNode == nullptr)
        return nullptr;

    if (valueToRemove < currentNode->data)
        currentNode->left = remove(valueToRemove, currentNode->left);
    else if (valueToRemove > currentNode->data)
        currentNode->right = remove(valueToRemove, currentNode->right);
    else if (currentNode->left && currentNode->right) { //Если текущий узел имеет два потомка, находим минимальный элемент в 
        //правом поддереве, копируем его значение в текущий узел и удаляем этот минимальный элемент:
        temp = findMinimal(currentNode->right);
        currentNode->data = temp->data;
        currentNode->right = remove(currentNode->data, currentNode->right);
    }
    else {//Если текущий узел имеет только одного потомка или не имеет потомков, 
        //заменяем текущий узел его потомком (если он есть) и удаляем текущий узел:
        temp = currentNode;
        if (currentNode->left == nullptr)
            currentNode = currentNode->right;
        else if (currentNode->right == nullptr)
            currentNode = currentNode->left;
        delete temp;
    }

    return currentNode;
}

void BinarySearchTree::inorder(Node* currentNode) { //нфиксный обход дерева, то есть посещает узлы в возрастающем порядке значений.
    if (currentNode == nullptr)
        return;
    inorder(currentNode->left);
    std::cout << currentNode->data << " ";
    inorder(currentNode->right);
}

BinarySearchTree::Node* BinarySearchTree::findMinimal(Node* currentNode) {
    if (currentNode == nullptr)
        return nullptr;
    else if (currentNode->left == nullptr)
        return currentNode;
    else
        return findMinimal(currentNode->left);
}

BinarySearchTree::Node* BinarySearchTree::findMaximum(Node* currentNode) {
    if (currentNode == nullptr)
        return nullptr;
    else if (currentNode->right == nullptr)
        return currentNode;
    else
        return findMaximum(currentNode->right);
}

BinarySearchTree::Node* BinarySearchTree::find(Node* currentNode, int valueToFind) {
    if (currentNode == nullptr)
        return nullptr;

    if (valueToFind < currentNode->data)
        return find(currentNode->left, valueToFind);
    else if (valueToFind > currentNode->data)
        return find(currentNode->right, valueToFind);
    else
        return currentNode;
}

BinarySearchTree::BinarySearchTree() {
    root = nullptr;
}

BinarySearchTree::~BinarySearchTree() {
    root = clear(root);
}

void BinarySearchTree::insert(int x) {
    root = insert(x, root);
}

void BinarySearchTree::remove(int x) {
    root = remove(x, root);
}

void BinarySearchTree::display() {
    inorder(root);
    std::cout << std::endl;
}

void BinarySearchTree::search(int x) {
    root = find(root, x);
}

int main() {
    BinarySearchTree tree;
    tree.insert(20);
    tree.insert(25);
    tree.insert(15);
    tree.insert(10);
    tree.insert(30);
    tree.display();
    tree.remove(20);
    tree.display();
    tree.remove(25);
    tree.display();
    tree.remove(30);
    tree.display();
    return 0;
}