#include<bits/stdc++.h>
#include"bot.h"
#include"socket.io-client-cpp_/build/include/sio_client.h"
using namespace std;
string custom="fafa";
string userid="<sdf>Mr.Dong";
string username="[Bot]KAZIKO";
int mp[N*N],cities[N*N],W,H,myindex,citysz;

void sleep_for(int ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
string tostr(int x){
	char s[20];
	sprintf(s,"%d",x);
	return s;
}

sio::message::list transdata(){
	return sio::message::list();
}
template<typename T,typename ...Args> 
sio::message::list transdata(T value,Args... rest){
	auto ret=transdata(rest...);
	ret.insert(0,value); 
	return ret;
}



int patch(int* a,vector<std::shared_ptr<sio::message> > diff){
	int s=0;
	for(int i=0;i<diff.size();){
		s+=diff[i].get()->get_int();
		i++;
		if(i>=diff.size())break;
		for(int j=1;j<=diff[i].get()->get_int();++j)
			a[s++]=diff[i+j].get()->get_int();
		i+=diff[i].get()->get_int()+1;
	}
	return s;
}
int inline get_soldier(int r,int c){
	return mp[r*W+c+2];
}
int inline get_mp(int r,int c){
	return mp[r*W+c+2+W*H];
}
int inline chkcity(int r,int c){
	for(int i=0;i<citysz;++i)if(cities[i]==r*W+c)
		return true;
	return false;
}


sio::message::list rndbot_sol(){
	while(1){
		int pos=rand()%(W*H),row=pos/W,col=pos%W;
		if(get_mp(row,col)==myindex){
			int rnd=rand()%4; 
			int endpos=pos;
			if(rnd==0&&col>0){
				endpos--;
			} else if(rnd==1&&col<W-1){
				endpos++;
			} else if(rnd==2&&row<H-1){
				endpos+=W;
			} else if(rnd==3&&row>0){
				endpos-=W;
			} else continue;
			return transdata(tostr(pos),tostr(endpos));
			break;
		}
	}
	assert(false);
}

sio::message::list cleverer_bot_sol(){
	
}

sio::message::list bot_sol(){
	return rndbot_sol();
}


void print_mp(){
	for(int i=0;i<citysz;++i)printf("%d ",cities[i]);puts("\n>>>>>>>>>>");
	for(int i=0;i<W*H;++i)printf("%d%c",mp[i+2],i%W==W-1?'\n':' ');puts("\n>>>>>>>>>>");
	for(int i=0;i<W*H;++i)printf("%d%c",mp[i+W*H+2],i%W==W-1?'\n':' ');puts("\n>>>>>>>>>>");
}

int main(){
	sio::client h;
	h.socket()->on("game_start",[&](sio::event& ev){
		auto data=ev.get_message().get()->get_map();
		myindex=data["playerIndex"].get()->get_flag();
		int replay_id=data["replay_id"].get()->get_flag();
		cout<<"Game start! Replay id:"<<replay_id<<" PlayerIndex: "<<myindex<<endl;
	});	
	h.socket()->on("error_set_username",[&](sio::event& ev){
		cout<<"Error on setting username"<<endl;
		auto data=ev.get_message().get()->get_string();
		cout<<data<<endl; 
	});
	
	h.socket()->on("game_update",[&](sio::event& ev){
		printf("OK,game_update\n");
		auto data=ev.get_messages();
		auto upd=data[0].get()->get_map();
		int turn=upd["turn"].get()->get_int();
		auto cities_diff=upd["cities_diff"].get()->get_vector();
		auto map_diff=upd["map_diff"].get()->get_vector();
		
		citysz=patch(cities,cities_diff);
		patch(mp,map_diff);
		W=mp[0],H=mp[1];
		
		
		printf("OK4(turn:%d,W:%d,H:%d)\n",turn,W,H);
		print_mp();
		h.socket()->emit("attack",bot_sol());
		
	});
	h.socket()->on("game_lost",[&](sio::event& ev){
		cout<<"Game is over. Lost."<<endl;
		h.socket()->emit("leave_game");
		h.socket()->emit("join_private",transdata(custom,userid));
	});
	h.socket()->on("game_won",[&](sio::event& ev){
		cout<<"Game is over. Win."<<endl;
		h.socket()->emit("leave_game");
		h.socket()->emit("join_private",transdata(custom,userid));
	});
	
	
	h.set_open_listener([&](){
		cout<<"Connected to server"<<endl;
		h.socket()->emit("set_username",transdata(userid,username));
		h.socket()->emit("join_private",transdata(custom,userid));
		sleep_for(500);
		cout<<"Joined custom game "<<custom<<endl;
	});
	
	h.connect("http://botws.generals.io");
	while(1){
		string cmd;
		cout<<">";
		cin>>cmd;
		if(cmd=="1"){
			h.socket()->emit("set_force_start",transdata(custom,sio::bool_message::create(true)));
			cout<<"Set force start."<<endl;
		}
	} 
} 
