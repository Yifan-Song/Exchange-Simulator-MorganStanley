#include <array>
#include <iostream>
#include <string>
#include <cstring>
#include <boost/asio.hpp>
#include <map>
#include <vector>
#include <algorithm>
#include <time.h>
using boost::asio::ip::tcp;

std::map<std::string, std::vector<std::string>> BuyBook;//���ڴ���orderbook�е�buyһ�������
std::map<std::string, std::vector<std::string>> SellBook;//���ڴ���orderbook�е�sellһ�������
std::map<std::string, std::vector<float>> BuyPrice;
std::map<std::string, std::vector<float>> SellPrice;
std::vector<std::string> Order;//���ڴ���ʵʱ���µ�orderʵ��
std::vector<std::string> OrderID;//���ڴ�������orderid
std::string OrdStatus;

bool isOpen()//�����жϵ���ʱ���г��Ƿ񿪷�
{
	using namespace std;
	time_t Edit;
	time(&Edit);
	tm gm;
	gmtime_s(&gm, &Edit);
	int hour = gm.tm_hour + 8;
	int min = gm.tm_min;
	if (hour >= 13 && hour < 15)return true;
	else if (hour == 15 && min <= 30)return true;
	else if (hour == 9 && min >= 30)return true;
	else if (hour == 10)return true;
	else if (hour == 11 && min <= 30)return true;
	else return false;
}

std::string Read(std::string s, std::string aim)//���ڷ���fixЭ����tag��Ӧ��ֵ������ֵsΪfixЭ�飬aimΪtag
{
	using namespace std;
	if (s.find(aim + "=") == s.npos)
	{
		return "-1";
	}
	int position = 0;
	int temp;
	string tempStr;
	string output;
	string temps = s;
	while (true)
	{
		int tempPosition;
		tempPosition = temps.find(aim);
		position += tempPosition;
		if (s[position + aim.length()] == '=')break;
		else
		{
			position++;
			temps = s.substr(position , s.length() - position);
		}
	}
	temp = s.find(';');
	while (true)
	{
		if (temp > position)break;
		else tempStr = s.substr(temp + 1, s.length() - temp);
		temp += tempStr.find(';');
		temp++;
	}
	output = s.substr(position, temp - position);
	output = output.substr(output.find('=') + 1, output.length() - output.find('='));
	return output;
}

std::string Transform(std::string s)//��orderbook�д洢��fixЭ������Ҫ��ѯʱת��Ϊ�ɶ���һ��������Ϣ
{
	using namespace std;
	string name = Read(s, "GoodName");
	int qty;
	float price;
	stringstream output;
	output << Read(s, "38");
	output >> qty;
	output.clear();
	output.str("");
	output << Read(s, "44");
	output >> price;
	output.clear();
	output.str("");
	output << qty << "\t" << price;
	return output.str();
}

void Bookin(std::string s)//�����յ���fixЭ���¼��orderbook��
{
	using namespace std;
	string name = Read(s, "GoodName");
	float price;
	stringstream temp;
	temp << Read(s, "44");
	temp >> price;
	if (s.find("40=1") != s.npos)
	{
		BuyPrice[name].push_back(price);
		sort(BuyPrice[name].begin(),BuyPrice[name].end());
		vector<float>::iterator position = find(BuyPrice[name].begin(), BuyPrice[name].end(), price);
		BuyBook[name].insert(BuyBook[name].begin() + (position - BuyPrice[name].begin()), s);
	}
	else
	{
		SellPrice[name].push_back(price);
		sort(SellPrice[name].begin(), SellPrice[name].end());
		vector<float>::iterator position = find(SellPrice[name].begin(), SellPrice[name].end(), price);
		SellBook[name].insert(SellBook[name].begin() + (position - SellPrice[name].begin()), s);
	}
}

std::string WriteBook(std::string name)//��name��Ӧ��good��orderbook���벢����
{
	using namespace std;
	std::stringstream message;
	message << "\n" << "\tGood Name:" << name << "\nBuy Orders\tSell Orders\nShares\tPrice\tShares\tPrice\n";
	int BuyBookNum = 10;
	int SellBookNum = 10;
	int min;
	int max;
	if (BuyBook[name].size() < 10)BuyBookNum = BuyBook[name].size();
	if (SellBook[name].size() < 10)SellBookNum = SellBook[name].size();
	if (BuyBookNum <= SellBookNum)
	{
		min = BuyBookNum;
		max = SellBookNum;
	}
	else
	{
		min = SellBookNum;
		max = BuyBookNum;
	}
	for (int i = 0;i < min;i++)
	{
		message << Transform(BuyBook[name][BuyBook[name].size()-i-1]) << "\t" << Transform(SellBook[name][i]) << "\n";
	}
	if (max > min)
	{
		if (max == BuyBookNum)
		{
			for (int i = min;i < max;i++)
			{
				message << Transform(BuyBook[name][BuyBook[name].size() - i - 1]) << "\n";
			}
		}
		else
		{
			for (int i = min;i < max;i++)
			{
				message << "\t\t" << Transform(SellBook[name][i]) << "\n";
			}
		}
	}
	message << "\n";
	return message.str();
}

std::string Change(std::string s, std::string tag, std::string aim)//�����޸�fixЭ����ĳtag��Ӧ��ֵ��sΪfixЭ�飬tagΪ��Ӧtag��aimΪ�޸ĵ�Ŀ��
{
	using namespace std;
	int position;
	string tempStr;
	string newStr;
	position = s.find(tag);
	newStr = s.substr(0, position);
	tempStr = s.substr(position, s.length() - position);
	newStr += tempStr.substr(0, tempStr.find("=") + 1);
	newStr += aim;
	newStr += tempStr.substr(tempStr.find(";"), tempStr.length() - tempStr.find(";"));
	return newStr;
}

std::vector<std::string> Deal(std::string name)//���н��ײ�����һ��vector�����а���Ҫ���ظ�client�ĸ�����Ϣ
{
	using namespace std;
	vector<string> sendBackMessages;
	if (BuyBook[name].size() == 0 || SellBook[name].size() == 0)return sendBackMessages;
	stringstream tempBuy;
	stringstream tempSell;
	stringstream temp;
	string sendBackMessage = "";
	int tempBuyQty;
	int tempSellQty;
	float tempBuyPrice;
	float tempSellPrice;
	tempBuy << Transform(BuyBook[name][BuyBook[name].size()-1]);
	tempSell << Transform(SellBook[name][0]);
	tempBuy >> tempBuyQty;
	tempBuy >> tempBuyPrice;
	tempSell >> tempSellQty;
	tempSell >> tempSellPrice;
	while(tempBuyPrice >= tempSellPrice)
	{
		temp.clear();
		temp.str("");
		tempBuy.clear();
		tempBuy.str("");
		tempSell.clear();
		tempSell.str("");
		if (tempBuyQty < tempSellQty)
		{
			temp << (tempSellQty - tempBuyQty);
			int position = find(Order.begin(), Order.end(), SellBook[name][0]) - Order.begin();
			SellBook[name][0] = Change(SellBook[name][0], "39", "1");
			SellBook[name][0] = Change(SellBook[name][0], "150", "1");
			SellBook[name][0] = Change(SellBook[name][0], "35", "8");
			SellBook[name][0] = Change(SellBook[name][0], "38", temp.str());
			Order[position] = SellBook[name][0];
			sendBackMessage = Order[position];
			sendBackMessages.push_back(sendBackMessage);
			position = find(Order.begin(), Order.end(), BuyBook[name][BuyBook[name].size() - 1]) - Order.begin();
			BuyBook[name][BuyBook[name].size() - 1] = Change(BuyBook[name][BuyBook[name].size() - 1], "35", "8");
			BuyBook[name][BuyBook[name].size() - 1] = Change(BuyBook[name][BuyBook[name].size() - 1], "39", "2");
			BuyBook[name][BuyBook[name].size() - 1] = Change(BuyBook[name][BuyBook[name].size() - 1], "150", "2");
			Order[position] = BuyBook[name][BuyBook[name].size() - 1];
			sendBackMessage = Order[position];
			sendBackMessages.push_back(sendBackMessage);
			BuyBook[name].erase(BuyBook[name].end()-1);
		}
		if (tempBuyQty == tempSellQty)
		{
			int position = find(Order.begin(), Order.end(), BuyBook[name][BuyBook[name].size() - 1]) - Order.begin();
			BuyBook[name][BuyBook[name].size() - 1] = Change(BuyBook[name][BuyBook[name].size() - 1], "35", "8");
			BuyBook[name][BuyBook[name].size() - 1] = Change(BuyBook[name][BuyBook[name].size() - 1], "39", "2");
			BuyBook[name][BuyBook[name].size() - 1] = Change(BuyBook[name][BuyBook[name].size() - 1], "150", "2");
			Order[position] = BuyBook[name][BuyBook[name].size() - 1];
			sendBackMessage = Order[position];
			sendBackMessages.push_back(sendBackMessage);
			position = find(Order.begin(), Order.end(), SellBook[name][0]) - Order.begin();
			SellBook[name][0] = Change(SellBook[name][0], "35", "8");
			SellBook[name][0] = Change(SellBook[name][0], "39", "2");
			SellBook[name][0] = Change(SellBook[name][0], "150", "2");
			Order[position] = SellBook[name][0];
			sendBackMessage = Order[position];
			sendBackMessages.push_back(sendBackMessage);
			BuyBook[name].erase(BuyBook[name].end()-1);
			SellBook[name].erase(SellBook[name].begin());
		}
		if (tempBuyQty > tempSellQty)
		{
			temp << (tempBuyQty - tempSellQty);
			int position = find(Order.begin(), Order.end(), SellBook[name][0]) - Order.begin();
			SellBook[name][0] = Change(SellBook[name][0], "35", "8");
			SellBook[name][0] = Change(SellBook[name][0], "39", "2");
			SellBook[name][0] = Order[position] = Change(SellBook[name][0], "150", "2");
			Order[position] = SellBook[name][0];
			sendBackMessage = Order[position];
			sendBackMessages.push_back(sendBackMessage);
			position = find(Order.begin(), Order.end(), BuyBook[name][BuyBook[name].size() - 1]) - Order.begin();
			BuyBook[name][BuyBook[name].size() - 1] = Change(BuyBook[name][BuyBook[name].size() - 1], "35", "8");
			BuyBook[name][BuyBook[name].size() - 1] = Change(BuyBook[name][BuyBook[name].size() - 1], "39", "1");
			BuyBook[name][BuyBook[name].size() - 1] = Change(BuyBook[name][BuyBook[name].size() - 1], "150", "1");
			BuyBook[name][BuyBook[name].size() - 1] = Change(BuyBook[name][BuyBook[name].size() - 1], "38", temp.str());
			Order[position] = BuyBook[name][BuyBook[name].size() - 1];
			sendBackMessage = Order[position];
			sendBackMessages.push_back(sendBackMessage);
			SellBook[name].erase(SellBook[name].begin());
		}
		if (BuyBook[name].size() == 0 || SellBook[name].size() == 0)return sendBackMessages;
		tempBuy << Transform(BuyBook[name][BuyBook[name].size() - 1]);
		tempSell << Transform(SellBook[name][0]);
		tempBuy >> tempBuyQty;
		tempBuy >> tempBuyPrice;
		tempSell >> tempSellQty;
		tempSell >> tempSellPrice;
	}
	return sendBackMessages;
}

std::string ProcessNewOrder(std::string visitor)
{
	std::cout << "New Order accepted.\n";
	std::string message;
	std::string dealMessage;
	message = visitor;
	message += "39=0;150=0;";
	message = Change(message, "35", "8");
	Bookin(message);
	Order.push_back(message);
	OrderID.push_back(Read(message, "11"));
	return message;
}

std::string ProcessCancelOrder(std::string visitor)
{
	std::string sendBackMessage = "11=";
	sendBackMessage += Read(visitor, "11");
	sendBackMessage += ";";
	if (find(OrderID.begin(), OrderID.end(), Read(visitor, "11")) == OrderID.end())
	{
		sendBackMessage += "35=9;39=8;";
		return sendBackMessage;
	}
	else
	{
		std::string message = Order[find(OrderID.begin(), OrderID.end(), Read(visitor, "11")) - OrderID.begin()];
		if (Read(message, "39") == "0" || Read(message, "39") == "1")
		{
			std::string name = Read(message, "GoodName");
			if (Read(message, "40") == "1")BuyBook[name].erase(find(BuyBook[name].begin(), BuyBook[name].end(), message));
			else SellBook[name].erase(find(SellBook[name].begin(), SellBook[name].end(), message));
			message = Change(message, "35", "8");
			message = Change(message, "39", "4");
			message = Change(message, "150", "4");
			Order[find(OrderID.begin(), OrderID.end(), Read(visitor, "11")) - OrderID.begin()] = message;
			sendBackMessage += "35=8;150=4;39=4;";
			return sendBackMessage;
		}
		else
		{
			sendBackMessage += "35=9;39=8;";
			return sendBackMessage;
		}
	}
}

int main()
{
	if (!isOpen())
	{
		std::cout << "The market is closed now.\nOpentime:9:30AM-11:30AM && 1:00PM-3:30PM\n";//��ʱ�䲻�Ϸ�ʱֱ���˳�
		return 0;
	}
	boost::asio::io_service io_service;
	tcp::acceptor acc(io_service, tcp::endpoint(tcp::v6(), 9876));

	boost::asio::io_service io_service1;
	tcp::acceptor acc1(io_service1, tcp::endpoint(tcp::v6(), 9875));
	while (true)
	{
		std::cout << "Waiting for client to connect...\n";//�ȴ��ͻ�������
		boost::system::error_code ignored;
		tcp::socket socket(io_service);
		acc.accept(socket);
		std::array<char, 256> input_buffer;
		std::cout << "Client connected successfully.\n";
		std::cout << "Waiting for monitor client to connect...\n";
		boost::asio::write(socket, boost::asio::buffer("Waiting for monitor client to connect...\n"), ignored);//�ȴ����ӿͻ������Ӳ���֪�ͻ�����Ҫ�ȴ�
		boost::system::error_code ignored1;
		tcp::socket socket1(io_service1);
		acc1.accept(socket1);
		std::array<char, 256> input_buffer1;
		std::size_t input_size1 = socket1.read_some(
			boost::asio::buffer(input_buffer1),
			ignored1);
		std::string visitor1(input_buffer1.data(),
			input_buffer1.data() + input_size1);
		std::cout << "Monitor client connected successfully.\nTrading...\n";
		boost::asio::write(socket, boost::asio::buffer("Market is open.\n"), ignored);//��֪�ͻ���׼�����
		while (true)
		{
			std::size_t input_size = socket.read_some(
				boost::asio::buffer(input_buffer),
				ignored);
			std::string visitor(input_buffer.data(),
				input_buffer.data() + input_size);
			socket.remote_endpoint().address().to_string();
			
			if (!isOpen())
			{
				std::cout << "The market is closed now.\nOpentime:9:30AM-11:30AM && 1:00PM-3:30PM\n";//�ڽ���ʱ���ⲻ�����¶���
				system("pause");
			}
			boost::asio::write(socket1, boost::asio::buffer(visitor), ignored1);

			if (visitor.find("QueryName") != visitor.npos)
			{
				boost::asio::write(socket, boost::asio::buffer(WriteBook(Read(visitor, "QueryName"))), ignored);//�����ѯorderbook�����
			}
			if (visitor.find("35=D") != visitor.npos)//��������¶��������
			{
				std::string sendBackMessage = ProcessNewOrder(visitor);
				boost::asio::write(socket, boost::asio::buffer(sendBackMessage), ignored);
				boost::asio::write(socket1, boost::asio::buffer(sendBackMessage), ignored1);
				Sleep(50);
				std::vector<std::string> dealMessages = Deal(Read(visitor, "GoodName"));//���н���
				dealMessages.push_back("End");
				for (std::string message : dealMessages)
				{
					boost::asio::write(socket, boost::asio::buffer(message), ignored);
					boost::asio::write(socket1, boost::asio::buffer(message), ignored1);
					Sleep(50);//��ֹ������Ϣ������̵��¶�����Ϣ����һ�𱻽��ܵ����
				}
			}
			if (visitor.find("35=F") != visitor.npos)//�������ȡ����������������
			{
				std::string sendBackMessage = ProcessCancelOrder(visitor);
				boost::asio::write(socket, boost::asio::buffer(sendBackMessage), ignored);
				boost::asio::write(socket1, boost::asio::buffer(sendBackMessage), ignored1);
			}
		}
		socket.shutdown(tcp::socket::shutdown_both, ignored);
		socket.close();
		socket1.shutdown(tcp::socket::shutdown_both, ignored1);
		socket1.close();
	}
	return 0;
}