#include<stdio.h>
#include<iostream>
#include<vector>
#include<string>
#include<fstream>
#include<map>
#include<iomanip>
using namespace std;

class Node{
public:
	 Node() = default;
	 Node(string value):value(value), pre(NULL), next(NULL){}
	 ~Node() = default;

	string value;//page number
	Node* pre;
	Node* next;
};
class Cache{
public:
	Cache(){
		head = new Node();
		tail = new Node();
		
		head->next = tail;
		head->pre = NULL;

		tail->pre = head;
		tail->next = NULL;
		size = 0;
	}
	~Cache(){
		delete head;
		table.clear();
	}

	Node* FIND(string page){
		map<string, Node*>::iterator it;
		it = table.find(page);

		if(it==table.end())
			return NULL;
		else
			return it->second;
	}

	void PUT(string page){
		Node* node = new Node(page);
		//node->value = page;
		node->next = head->next;
		head->next->pre = node;
		head->next = node;
		node->pre = head;

		//table.insert(std::make_pair<string, Node*>(page, node));
		table[page] = node;
		++size;
		return;
	}

	void remove_tail(){

		Node* tmp = tail->pre;
		tmp->pre->next = tail;
		tail->pre = tmp->pre;

		table.erase(tmp->value);
		
		tmp->next = NULL;
		tmp->pre = NULL;
		delete tmp;
		--size;
		return;
	}

	void move_front(Node* node){
		node->pre->next = node->next;
		node->next->pre = node->pre;

		node->next = head->next;
		head->next->pre = node;
		head->next = node;
		node->pre = head;

		return;
	}
	
	void show(){//debug function
		for(Node* tmp=head->next;tmp->next!=NULL;tmp=tmp->next){
			cout << tmp->value << "  ";
		}
		cout << endl;
		return;
	}
	
	int size;
	map<string, Node*> table;
	Node* head;
	Node* tail;
};

int main(){
	ifstream file;
	long long int miss, hit;
	string tmp;
	Node* cur;
	file.open("trace.txt", ios::in);

	if(!file){
		cout << "can't open trace.txt\n";
		exit(1);
	}

	cout << "FIFO---\n";
	cout << "size\t\tmiss\t\thit\t\tpage fault ratio\n";
	for(int i=64;i<=512;i*=2){
		miss = 0;
		hit = 0;
		Cache fifo;
		while(getline(file, tmp)){
			string page(tmp, 3, 5);//page = tmp(3,7);
			page += '\0';

			cur = fifo.FIND(page);

			if(cur==NULL){
				++miss;
				if(fifo.size == i){
					fifo.remove_tail();
					fifo.PUT(page);
					//fifo.show();
				}

				else
					fifo.PUT(page);
			}

			else
				++hit;

		}
		cout << i << "\t\t" << miss << "\t\t" << hit << "\t" << fixed << setprecision(9)<<(double)miss/(double)(miss+hit) << "\n";
		file.clear();
		file.seekg(0);
	}


	cout<<"\nLRU---\n";
	cout << "size\t\tmiss\t\thit\t\tpage fault ratio\n";
	for(int i=64;i<=512;i*=2){
		Cache LRU;
		miss = 0;
		hit = 0;
		//while(file.getline(tmp, 1024, '\n')){
		while(getline(file, tmp)){
			string page(tmp, 3, 5);
			page += '\0';

			cur = LRU.FIND(page);

			if(cur==NULL){
				++miss;

				if(LRU.size == i){
					LRU.remove_tail();
					LRU.PUT(page);
				}

				else
					LRU.PUT(page);
			}

			else{
				++hit;
				
				LRU.move_front(cur);
			}

		}
		cout << i << "\t\t" << miss << "\t\t" << hit << "\t" << fixed << setprecision(9)<<(double)miss/(double)(miss+hit) << "\n";
		file.clear();
		file.seekg(0, file.beg);
	}
	
	file.close();
	return 0;
}