/*
 * Sun Yat-sen University, School of Data Science and Computer.
 * Information and Computing Science. Class One.
 * 16339028 Liu Yuan. Email: ryanliu2015@outlook.com.
 * (operand) 2018 RyanLiuFTZ. All rights reserved.
 * Operating System Project 6: Banker'command_type Algorithm.
 */

#include <iostream>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

/*
 * Based on the example in
 * Operating System Concepts (7th Edition).
 * Page 261, 7.5.3.3: An Illustrative Example.
 */

struct Resource
{
	int prev;
	int curr;
};

struct Process
{
	int process_id;
	int resource_id;
	int maximum;
	int holding;
};

struct Command
{
	string command_type;
	int process_id, resource_id, operand, assign;
	Command(string in_command_type, int in_process_id, int in_resource_id, int in_operand, int in_assign) :
		command_type(in_command_type),
		process_id(in_process_id),
		resource_id(in_resource_id),
		operand(in_operand),
		assign(in_assign){}
};

struct Resource* resource;
struct Process* process;
int process_num, resource_num;
vector<struct Command>* all_command;
int* run_time;
int* wait_time;
int process_count;

void init_command(int process_id, int resource_id, int operand)
{
	int index = (process_id - 1) * resource_num + resource_id;
	if (operand <= resource[resource_id].prev)
	{
		process[index].maximum = operand;
		++run_time[process_id];
		all_command[process_id].erase(all_command[process_id].begin());
	}
	else
	{
		throw operand;
		++run_time[process_id];
	}
}

void process_request(int process_id, int resource_id, int operand)
{
	int index = (process_id - 1) * resource_num + resource_id;
	if (operand + process[index].holding <= process[index].maximum)
	{
		if (operand <= resource[resource_id].prev)
		{
			process[index].holding += operand;
			resource[resource_id].prev -= operand;
			++run_time[process_id];
			all_command[process_id].erase(all_command[process_id].begin());
		}
		else
		{
			int u1 = resource[resource_id].prev;
			int u3 = process[index].holding;
			operand -= resource[resource_id].prev;
			process[index].holding += resource[resource_id].prev;
			resource[resource_id].prev = 0;
			bool tf = 0;
			int temp[1000];
			for (int i = process_num; i > process_id; --i)
			{
				temp[i] = process[(i - 1) * resource_num + resource_id].holding;
			}
			for (int i = process_num; i > process_id; --i)
			{
				int index_2 = (i - 1) * resource_num + resource_id;
				if (process[index_2].holding >= operand)
				{
					process[index_2].holding -= operand;
					process[index].holding += operand;
					++run_time[process_id];
					tf = 1;
					all_command[process_id].erase(all_command[process_id].begin());
					break;
				}
				else
				{
					process[index].holding += process[index_2].holding;
					operand -= process[index_2].holding;
					process[index_2].holding = 0;
				}
			}
			if (!tf)
			{
				resource[resource_id].prev = u1;
				process[index].holding = u3;
				for (int i = process_num; i > process_id; --i)
				{
					process[(i - 1)*resource_num + resource_id].holding = temp[i];
				}
				++wait_time[process_id];
				++run_time[process_id];
			}
		}
	}
	else
	{
		throw operand;
	}
}

void process_relsase(int process_id, int resource_id, int operand)
{
	int index = (process_id - 1) * resource_num + resource_id;
	if (operand > process[index].holding)
	{
		int tem = operand - process[index].holding;
		++wait_time[process_id];
		++run_time[process_id];
		struct Command temp("request+release", process_id, resource_id, operand, tem);
		all_command[process_id].erase(all_command[process_id].begin());
		all_command[process_id].insert(all_command[process_id].begin(), temp);
	}
	else
	{
		if (resource[resource_id].curr == -1)
			resource[resource_id].curr = resource[resource_id].prev + operand;
		else
			resource[resource_id].curr += operand;
		process[index].holding -= operand;
		++run_time[process_id];
		all_command[process_id].erase(all_command[process_id].begin());
	}
}

void process_terminate(int process_id, int resource_id, int operand)
{
	for (int i = 0; i < resource_num; ++i)
	{
		if (resource[i + 1].curr == -1)
			resource[i + 1].curr = resource[i + 1].prev + process[(process_id - 1)*resource_num + i + 1].holding;
		else
			resource[i + 1].curr += process[(process_id - 1)*resource_num + i + 1].holding;
		process[(process_id - 1) * resource_num + i + 1].holding = 0;
	}
	++process_count;
	all_command[process_id].erase(all_command[process_id].begin());
}

void release_2(int process_id, int resource_id, int operand, int assign)
{
	int index = (process_id - 1)*resource_num + resource_id;
	if (assign + process[index].holding <= process[index].maximum)
	{
		if (assign <= resource[resource_id].prev)
		{
			process[index].holding += assign;
			resource[resource_id].prev -= assign;
			process_relsase(process_id, resource_id, operand);
		}
		else
		{
			int u1 = resource[resource_id].prev;
			int u3 = process[index].holding;
			assign -= resource[resource_id].prev;
			process[index].holding += resource[resource_id].prev;
			resource[resource_id].prev = 0;
			bool tf = 0;
			int temp[1000];
			for (int i = process_num; i > process_id; --i)
			{
				temp[i] = process[(i - 1) * resource_num + resource_id].holding;
			}
			for (int i = process_num; i > process_id; --i)
			{
				int index_2 = (i - 1) * resource_num + resource_id;
				if (process[index_2].holding >= assign)
				{
					process[index_2].holding -= assign;
					process[index].holding += assign;
					process_relsase(process_id, resource_id, operand);
					tf = 1;
					break;
				}
				else
				{
					process[index].holding += process[index_2].holding;
					assign -= process[index_2].holding;
					process[index_2].holding = 0;
				}
			}
			if (!tf)
			{
				resource[resource_id].prev = u1;
				process[index].holding = u3;
				for (int i = process_num; i > process_id; --i)
				{
					process[(i - 1)*resource_num + resource_id].holding = temp[i];
				}
				++wait_time[process_id];
				++run_time[process_id];
			}
		}
	}
	else
	{
		throw operand;
	}
}

int main()
{
	ifstream read_file("2.txt");
	read_file >> process_num >> resource_num;
	resource = new struct Resource[resource_num + 1];
	process = new struct Process[process_num * resource_num + 1];
	all_command = new vector<struct Command>[process_num + 1];
	run_time = new int[process_num + 1];
	wait_time = new int[process_num + 1];
	int index = 1;
	for (int i = 0; i < process_num; ++i)
	{
		run_time[i + 1] = wait_time[i + 1] = 0;
		for (int j = 0; j < resource_num; ++j)
		{
			process[index].process_id = i + 1;
			process[index].resource_id = j + 1;
			process[index].holding = 0;
			process[index].maximum = 0;
			++index;
		}
	}
	for (int i = 0; i < resource_num; ++i)
	{
		read_file >> resource[i + 1].prev;
		resource[i + 1].curr = -1;
	}
	string tmp_str;
	while (read_file >> tmp_str && tmp_str != "end")
	{
		int process_id, resource_id, operand;
		read_file >> process_id >> resource_id >> operand;
		struct Command temp(tmp_str, process_id, resource_id, operand, -1);
		all_command[process_id].push_back(temp);
	}
	read_file.close();

	while (process_count < process_num)
	{
		for (int i = 0; i < process_num; ++i)
		{
			if (!all_command[i + 1].empty())
			{
				struct Command temp = all_command[i + 1].front();
				if (temp.command_type == "initiate")
				{
					try
					{
						init_command(temp.process_id, temp.resource_id, temp.operand);
					}
					catch (int)
					{
						// not enough resources
						process_terminate(temp.process_id, temp.resource_id, temp.operand);
						int x = 0;
						while (!all_command[temp.process_id].empty())
						{
							all_command[temp.process_id].erase(all_command[temp.process_id].begin() + x);
							++x;
						}
						run_time[temp.process_id] = wait_time[temp.process_id] = -1;
					}
				}
				else if (temp.command_type == "request")
				{
					try
					{
						process_request(temp.process_id, temp.resource_id, temp.operand);
					}
					catch (int)
					{
						process_terminate(temp.process_id, temp.resource_id, temp.operand);
					}
				}
				else if (temp.command_type == "release")
				{
					process_relsase(temp.process_id, temp.resource_id, temp.operand);
				}
				else if (temp.command_type == "terminate")
				{
					process_terminate(temp.process_id, temp.resource_id, temp.operand);
				}
				else {
					int tem = temp.operand - process[(temp.process_id - 1) * resource_num + temp.resource_id].holding;
					release_2(temp.process_id, temp.resource_id, temp.operand, tem);
				}
			}
		}
		for (int i = 0; i < resource_num; ++i)
		{
			if (resource[i + 1].curr != -1)
			{
				resource[i + 1].prev = resource[i + 1].curr;
			}
		}
		for (int i = 0; i < resource_num; ++i)
		{
			resource[i + 1].curr = -1;
		}
	}

	ofstream write_file;
	write_file.open("output.txt");
	for (int i = 0; i < process_num; ++i)
	{
		double portion = 1.0 * wait_time[i + 1] / run_time[i + 1];
		int percentage = portion * 100;
		int rounded = portion * 1000;
		if (rounded % 10 >= 5)
		{
			percentage++;
		}
		if (run_time[i + 1] == -1 || wait_time[i + 1] < wait_time[i])
		{
			write_file << "Task " << i + 1 << ": aborted\n";
			cout << "Task " << i + 1 << ": aborted\n";
		}
		else
		{
			write_file << "Task " << i + 1 << ": " << run_time[i + 1]
				<< " " << wait_time[i + 1] << " " << percentage << "%\n";
			cout << "Task " << i + 1 << ": " << run_time[i + 1]
				<< " " << wait_time[i + 1] << " " << percentage << "%\n";
		}
	}
	write_file.close();

	delete [] resource;
	delete [] run_time;
	delete [] wait_time;
	delete [] process;
	delete [] all_command;

	system("pause");
	return 0;
}