#pragma once
#include <chrono>
#include <stdlib.h>
#include <cmath>
#include "Judge.h"
#include <math.h>
using namespace std;
// todo 思考对于根节点是否需要特判
class Node{
    public:
    int num_visits;
    Node* parent;
    Node* children[13];
    int expandables[13]; // 可以扩展的列号
    int expandables_cnt;
    int my_x; // my move
    int my_y; // my move
    double score;
    int** board; // 不行，发现还是要拷贝
    int* top; // 不行 发现还是要拷贝
    int M;
    int N;
    int is_over;
    int is_tie;
    int player;
    int weight_9[9]={4,5,6,7,8,7,6,5,4};
    int weight_10[10]={4,5,6,7,8,8,7,6,5,4};
    int weight_11[11]={5,6,7,8,9,10,9,8,7,6,5};
    int weight_12[12]={5,6,7,8,9,10,10,9,8,7,6,5};
    int weight_9_sum=52;
    int weight_10_sum=60;
    int weight_11_sum=80;
    int weight_12_sum=90;
    Node(int M,int N,int* top,int** board,Node* parent){
        this->M=M;
        this->N=N;
        //this->top=top;
        this->top=new int[N];
        for(int i=0;i<N;i++){
            this->top[i]=top[i];
        }
        this->board=new int*[M];
        for(int i=0;i<M;i++){
            this->board[i]=new int[N];
        }
        //this->board=board;
        for(int i=0;i<M;i++){
            for(int j=0;j<N;j++){
                this->board[i][j]=board[i][j];
            }
        }
        this->parent=parent;
        this->num_visits=0;
        this->score=0.0;
        this->is_over=0;
        this->is_tie=0;
        if(parent){
        this->player=3-parent->player;
        }else{
            this->player=1;
        }
        this->my_x=-1;
        this->my_y=-1;
        int cnt=0;
        for(int i=0;i<N;i++){
            if(top[i]>0){
                this->expandables[cnt]=i;
                cnt++;
            }
        }expandables_cnt=cnt;
        for(int i=0;i<N;i++){
            this->children[i]=nullptr;
        }
    }
    int my_rand(){
        // 列数为N 取对应的那一行权重
        int expandables_quan[13]={0};
        int *ptr;
        if(N==9){
            ptr=weight_9;
        }else if(N==10){
            ptr=weight_10;
        }else if(N==11){
            ptr=weight_11;
        }else{
           ptr=weight_12;
        }
        // 求 expandables 里面的对应下标
        int sum=0;
        for(int i=0;i<expandables_cnt;i++){
            expandables_quan[i]=ptr[expandables[i]];
            sum+=expandables_quan[i];
        }
        int r=rand()%sum;
        // 求满足的下标
        int tmp=0;
        for(int i=0;i<expandables_cnt;i++){
            tmp+=expandables_quan[i];
            if(tmp>=r){
                return i;
            }
        }// this statement is never reached
        return expandables_cnt/2;
    }
    ~Node(){
        for(int i=0;i<M;i++){
            delete[] this->board[i];
        }delete[] this->board;
        delete[] this->top;
        for(int i=0;i<N;i++){
            if(this->children[i]!=nullptr){
                delete this->children[i];
                this->children[i]=nullptr;
            }
        }
    }
};
class MCTSTree{
    public:
    MCTSTree(int M,int N,const int* top,int **board){
        this->M=M;
        this->N=N;
        for (int i=0;i<N;i++){
            this->top[i]=top[i];
        }
        this->board=board;
        this->root=new Node(M,N,this->top,this->board,0);
        this->root->player=1;
        this->num_nodes=0;
        this->num_leafs=0;
        for(int i=0;i<N;i++){
            this->weights[i]=0;
        }
    }
    ~MCTSTree(){
        delete this->root;
    }
    Node* UCTSearch(){
        // init clock
        const std::chrono::_V2::system_clock::time_point now = std::chrono::system_clock::now();
        int cnt=0;
        while(cnt<1000000){
            // check time
            const std::chrono::_V2::system_clock::time_point now2 = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = now2 - now;
            if (elapsed_seconds.count()>2.2){
                break;
            }
           /* if(cnt%100==0){
                printf("before treePolicy\n");
                fflush(stdout);
            }*/
            Node* v1=this->treePolicy(root);
            /*if(cnt%100==0){
                printf("before default Policy\n");
                fflush(stdout);
            }*/
            //printf("before defaultPolicy\n");
                double default_policy_result=this->defaultPolicy(v1); 
            /*    if(cnt%100==0){
                    printf("before backup\n");
                    fflush(stdout);
                }*/
                //printf("before backup\n");
                this->backup(v1,default_policy_result);
            cnt++;
        }
            return this->bestRootChild();
        }
    
    Node* expand(Node* node){ // 参数传入 Node* 是反编译代码中的a2
    // 随机选取一列进行扩展
    int v1 = node->my_rand();
    // 先拷贝一份 top
    int top[13];
    for(int i=0;i<node->N;i++){
        top[i]=node->top[i];
    }
    int idx=node->expandables[v1]; // 列号
    int pos=node->top[idx]-1; // 行号 注意，按照pdf上的图，认为列从上往下增长
    top[idx]--;
    // 回避掉挖去的点
    if((pos-1>=0)&&(node->board[pos-1][idx]==-1)){
        top[idx]--;
    }
    node->children[idx]=new Node(this->M,this->N,top,node->board,node);
    node->children[idx]->my_x=pos;
    node->children[idx]->my_y=idx;
    //printf("in expand inserting point..%d %d player: %d\n",pos,idx,node->player);
    node->children[idx]->board[pos][idx]=node->children[idx]->player;
    // update expandable nodes
    //printf("in expand updating expandables_cnt %d\n",node->expandables_cnt);
    int tmp=node->expandables[node->expandables_cnt-1];
    node->expandables[v1]=tmp;
    node->expandables_cnt--;
    return node->children[idx];
    }
    void backup(Node* node,double result){
        while(node!=nullptr){
            node->num_visits++;
            node->score+=result;
            node=node->parent;
        }
    }
    Node* treePolicy(Node* node){
        // judge if mode is terminal
        // 情况：1. 棋盘满了 2. 有一方赢了
        while(true){
        int is_terminal=0;
        if(node->parent!=nullptr){
        if(node->is_over){
            is_terminal=1;
        }else{
            if(node->is_tie){
                is_terminal=1;
            }else{
                if(isTie(this->N,node->top)){
                    is_terminal=1;
                    node->is_tie=1;
                }else{
                    if(node->player==1&&userWin(node->my_x,node->my_y,node->M,node->N,node->board)){
                        is_terminal=1;
                        node->is_over=1;
                    }else if(node->player==2&&machineWin(node->my_x,node->my_y,node->M,node->N,node->board)){
                        is_terminal=1;
                        node->is_over=1;
                    }
                }
            }
        }
        }
        if(is_terminal){
            return node;
        }
        //judge if expanded
        if(node->expandables_cnt==0){
            node=bestChild(node);
        }else{
            return expand(node);
        }
        }
    }
    double defaultPolicy(Node* node){
        // judge if node is terminal
        // copy board
        int** board=new int*[node->M];
        for(int i=0;i<node->M;i++){
            board[i]=new int[node->N];
            for(int j=0;j<node->N;j++){
                board[i][j]=node->board[i][j];
            }
        }
        // top 也要拷贝
        int top[13];
        for(int i=0;i<node->N;i++){
            top[i]=node->top[i];
        }
        int my_player=node->player;
        double score=-2.0;
        if(node->player==1&&userWin(node->my_x,node->my_y,node->M,node->N,node->board)){
            score=-1.0;
        }else if(node->player==2&&machineWin(node->my_x,node->my_y,node->M,node->N,node->board)){
                score=1.0;
        }else if(isTie(node->N,node->top)){
            score=0.0;
            }
        if(score!=-2.0){
            for(int i=0;i<node->M;i++){
                delete[] board[i];}
                delete[] board;
            return score;
        }
        // continue simulate the tree
        // 随机选择一列进行扩展
        while(1){
        int pos=-1;
        int idx=0;
        while(pos<0){
        int v1 = node->my_rand();
        idx=node->expandables[v1]; // 列号
        pos=top[idx]-1; // 行号 注意，按照pdf上的图，认为列从上往下增长
        }
        top[idx]--;
        // 回避掉挖去的点
        if((pos-1>=0)&&(node->board[pos-1][idx]==-1)){
            top[idx]--;
            }
        // 赋值
        //printf("in defaultPolicy inserting point..%d %d player: %d\n",pos,idx,3-my_player);
        my_player=3-my_player;
        board[pos][idx]=my_player;
        if(my_player==2&&machineWin(pos,idx,node->M,node->N,board)){
            for(int i=0;i<node->M;i++){
                delete[] board[i];}
                delete[] board;
            return 1.0;
        }else if(my_player==1&&userWin(pos,idx,node->M,node->N,board)){
            for(int i=0;i<node->M;i++){
                delete[] board[i];}
                delete[] board;
            return -1.0;
        }else if (isTie(node->N,top)){
            for(int i=0;i<node->M;i++){
                delete[] board[i];}
                delete[] board;
            return 0.0;
        }
        }
        for(int i=0;i<node->M;i++){
            delete[] board[i];}
            delete[] board;
        return 0; // this statement is never reached
    }
    Node* bestChild(Node* node){
        double max_score=-10000000.0;
        Node* best_child=nullptr;
        for(int i=0;i<node->N;i++){
            if(node->children[i]!=nullptr){
                double tmp_score=node->children[i]->score/node->children[i]->num_visits*(node->children[i]->player==2?1:-1)+sqrt(2.0*log(double(node->num_visits))/double(node->children[i]->num_visits)); //注意用户出手是负的，要改回来
                if(tmp_score>max_score){
                    max_score=tmp_score;
                    best_child=node->children[i];
                }
            }
        }return best_child;
    }
    Node* bestRootChild(){
        double max_score=-1000000.0;
        Node* best_child=nullptr;
        for(int i=0;i<N;i++){
            if(root->children[i]!=nullptr){
                double tmp_score=0.5*root->children[i]->score*2.0/root->children[i]->num_visits;
                if(tmp_score>max_score){
                    max_score=tmp_score;
                    best_child=root->children[i];
                }
            }
        }
        //printf("best score: %f\n",max_score);
        return best_child;
    }
    Node* root;
    int M;
    int N;
    int top[13];
    int** board;
    int num_nodes;
    int num_leafs;
    int weights[13]; //todo 优化的时候再做吧
    
};