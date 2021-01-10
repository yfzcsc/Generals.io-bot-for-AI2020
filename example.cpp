#include<torch/torch.h>
#include<bits/stdc++.h>
using namespace torch;
using namespace std;
const int TILE_EMPTY=-1; 
const int TILE_MOUNTAIN=-2;
const int TILE_FOG=-3;
const int TILE_FOG_OBSTACLE=-4;
const int N=500,WIDTH=10,HEIGHT=10,ACTIONSZ=4*WIDTH*HEIGHT;
const int dx[4]={0,0,1,-1},dy[4]={-1,1,0,0},dpos[4]={-1,1,WIDTH,-WIDTH};
const double MAXREWARD=10.0;
const double WINREWARD=10.0;
const double INVALIDWALK=-5.0;
const double DEFSOLDIER=2.0;
const double GETCAMP=4.0;
const double WALKCORRECTLY=-0.5;
class Game;
struct bot{
	bool virtual isstop(){}
	void virtual startgame(){}
	void virtual putreward(double r){} // called each round
	pair<int,int> virtual sol(int myindex,Game& g){} // called each round 
};

class Game{
public:
	int TURN;
	int A[N][N],soldier[N][N],iscamp[N][N],generals[N],W,H,n,tg[N],army[N],turn_count;
	Game(int W,int H,int n):W(W),H(H),n(n){}
public:
	const static int TURN_KING=2;
	const static int TURN_MAP=50;
	int whosee;
	/* A[i][j](map): if (i,j) is empty, TILE_EMPTY; 
	if (i,j) is mountain, TILE_MOUNTAIN; 
	if (i,j) is camp, TILE_MOUNTAIN; 
	if (i,j) is general, player id. 
	if (i,j) is someone's, player id.
	
	soldier[i][j](number of soldiers): if (i,j) is camp, the number of soldiers needed. otherwise, 0 or someone's soldiers.
	iscamp[i][j]: if (i,j) is camp.
	generals[A]: where's the player A's general
	army[A]: the number of soldiers player A have. 
	*/
	
	
	// tg[A]: if player A init
	// general: no TILE_MOUNTAIN, no iscamp
	// camp: TILE_MOUNTAIN, iscamp
	// mountain: TILE_MOUNTAIN, no iscamp
	// empty: TILE_EMPTY
	bot*v[N];
	int checkin(int r,int c){
		return r<0||c<0||r>=H||c>=W;
	}
	int cansee(int id,int r,int c){
		if(id==-1)return true;
		if(checkin(r,c))return false;
		return (r>0&&A[r-1][c]==id)||(c>0&&A[r][c-1]==id)||(r<H-1&&A[r+1][c]==id)||(c<W-1&&A[r][c+1]==id)||(A[r][c]==id);
	}
	int get_iscamp(int r,int c){
		return iscamp[r][c];
	}
	int get_mp(int id,int r,int c){
		if(checkin(r,c))return TILE_FOG;
		if(cansee(id,r,c))
			return A[r][c];
		if(A[r][c]==TILE_MOUNTAIN)return TILE_FOG_OBSTACLE;
		return TILE_FOG;
	}
	int get_soldier(int id,int r,int c){
		if(checkin(r,c))return -1;
		if(cansee(id,r,c))
			return soldier[r][c];
		return -1;
	}
	int get_mp(int id,int pos){
		return get_mp(id,pos/W,pos%W);
	}
	int get_soldier(int id,int pos){
		return get_soldier(id,pos/W,pos%W);
	}
	void act(int id,pair<int,int> p,double& reward){
		int bg=p.first;
		int ed=p.second;
		int armys=soldier[bg/W][bg%W]-1;
		soldier[bg/W][bg%W]=1;
		if(A[ed/W][ed%W]==id)soldier[ed/W][ed%W]+=armys;
		else if(soldier[ed/W][ed%W]<armys){
			if(iscamp[ed/W][ed%W])reward+=GETCAMP;
			else reward+=DEFSOLDIER;
			if(A[ed/W][ed%W]>=0)army[A[ed/W][ed%W]]-=soldier[ed/W][ed%W];
			army[id]-=soldier[ed/W][ed%W];
			A[ed/W][ed%W]=id;
			soldier[ed/W][ed%W]=armys-soldier[ed/W][ed%W];
		} else {
			army[id]-=armys;
			if(A[ed/W][ed%W]>=0)army[A[ed/W][ed%W]]-=armys;
			soldier[ed/W][ed%W]-=armys;
		}
	}
	void do_lost(int winner,int loser){
		for(int i=0;i<H;++i)
			for(int j=0;j<W;++j)
				if(A[i][j]==loser)
					A[i][j]=winner;
		army[winner]+=army[loser];
		army[loser]=0;
	}
	void doswitch(int player){
		whosee=player;
	}
	
public:
	bool check_move(int id,int bg,int ed){
		if(bg==ed||checkin(bg/W,bg%W)||checkin(ed/W,ed%W))return false;
		if(get_mp(id,bg)!=id||get_soldier(id,bg)<=0)return false;
		if(A[ed/W][ed%W]==TILE_MOUNTAIN&&iscamp[ed/W][ed%W]!=1)return false;
		if(abs(bg-ed)!=1&&abs(bg-ed)!=W)return false;
		if(abs(bg-ed)==1&&min(bg,ed)%W==W-1&&W!=1)return false;
		return true;
	}
	void print_mp(){
		for(int i=0;i<W*H;++i)printf("%d%c",get_mp(-1,i),i%W==W-1?'\n':' ');puts("\n>>>>>>>>>>");
		for(int i=0;i<W*H;++i)printf("%d%c",get_soldier(-1,i),i%W==W-1?'\n':' ');puts("\n>>>>>>>>>>");
		fflush(stdout);
	}
	int start(){
		static bool vis[N],lost[N];
		static pair<int,int> pos[N];
		static int Q[N],ls[N],rest;
		static double reward[N];
		rest=n;
		whosee=-1;
		for(int i=0;i<n;++i)assert(tg[i]),assert(generals[i]>=0);
		for(int i=0;i<n;++i)army[i]=1,lost[i]=0;
		for(int i=0;i<n;++i)soldier[generals[i]/W][generals[i]%W]=1;
		for(int i=0;i<n;++i)v[i]->startgame();
		for(turn_count=1;;turn_count++){
			for(int i=0;i<n;++i)vis[i]=0;
			for(int i=0;i<n;++i)reward[i]=0;
			
			for(int i=0;i<n;++i)if(!lost[i]){
				doswitch(i);
				pair<int,int>p=v[i]->sol(i,*this);
				if(check_move(i,p.first,p.second))
					pos[i]=p;
				else vis[i]=true;
				doswitch(-1);
			} else vis[i]=true;
			int r[N];
			for(int i=0;i<n;++i)r[i]=i;
			random_shuffle(r,r+n);
			for(int i=0;i<n;++i)if(!vis[r[i]]){
				if(check_move(r[i],pos[r[i]].first,pos[r[i]].second))
					act(r[i],pos[r[i]],reward[r[i]]);
			}
			for(int i=0;i<n;++i)
				if(vis[i])reward[i]+=INVALIDWALK;
				else reward[i]+=WALKCORRECTLY;
			// lose
			for(int i=0;i<n;++i)if(!lost[i])
				if(A[generals[i]/W][generals[i]%W]!=i){
					rest--;
					lost[i]=true;
					do_lost(A[generals[i]/W][generals[i]%W],i);
				}
			
			if(rest==1){
				TURN=turn_count;
				printf("Game end at turn:%d,",turn_count);
				for(int i=0;i<n;++i){
					v[i]->putreward(lost[i]?-WINREWARD:WINREWARD);
				}
//				print_mp();
				for(int i=0;i<n;++i)if(!lost[i])
					return i;
			}
			for(int i=0;i<n;++i)
				v[i]->putreward(reward[i]);
			for(int i=0;i<n;++i){
				TURN=turn_count;
				if(v[i]->isstop())
					return -1;
			}
			// soldier increase
			if(turn_count%TURN_MAP==0){
				for(int i=0;i<H;++i)
					for(int j=0;j<W;++j)
						if(A[i][j]>=0)soldier[i][j]++,army[A[i][j]]++;
			} 
			if(turn_count%TURN_KING==0){
				for(int i=0;i<H;++i)
					for(int j=0;j<W;++j)
						if(A[i][j]>=0&&iscamp[i][j])soldier[i][j]++,army[A[i][j]]++;
				for(int i=0;i<n;++i)soldier[generals[i]/W][generals[i]%W]++,army[i]++;
			}
			
		}
		int x[10]={0};
		for(int i=0;i<H;++i)for(int j=0;j<W;++j)
			if(A[i][j]>=0)x[A[i][j]]+=soldier[i][j];
		for(int i=0;i<n;++i)printf("[General %d's army: %d,%d]\n",i,army[i],x[i]);
	}
	void generalinit(){
		for(int i=0;i<H;++i)
			for(int j=0;j<W;++j)
				A[i][j]=soldier[i][j]=iscamp[i][j]=0;
		for(int i=0;i<n;++i)tg[i]=army[i]=0,generals[i]=-1;
		whosee=-1;
	}
	void rndinit(int mountains,int camps){
		assert(mountains+camps+n<=W*H);
		generalinit();
		for(int i=0;i<H;++i)
			for(int j=0;j<W;++j)
				A[i][j]=TILE_EMPTY;
		vector<int> v;
		for(int i=0;i<W*H;++i)v.push_back(i);
		random_shuffle(v.begin(),v.end());
		for(int i=0;i<mountains;++i)
			A[v[i]/W][v[i]%W]=TILE_MOUNTAIN;
			
		random_shuffle(v.begin(),v.end());
		for(int i=0;i<camps;++i){
			if(A[v[i]/W][v[i]%W]==TILE_MOUNTAIN) continue;
			A[v[i]/W][v[i]%W]=TILE_MOUNTAIN;
			iscamp[v[i]/W][v[i]%W]=1;
			soldier[v[i]/W][v[i]%W]=40+rand()%10;
		}
		
		random_shuffle(v.begin(),v.end());
		for(int i=0,j=0;i<n;++i){
			while(A[v[j]/W][v[j]%W]==TILE_MOUNTAIN) j++;
			A[v[j]/W][v[j]%W]=i;
			generals[i]=v[j];
			j++;
		}
	}
	void playerinit(int id,bot* func){
		assert(id>=0&&id<n);
		tg[id]=1,v[id]=func;
	}
	int getmp(int r,int c){
		return get_mp(whosee,r,c);
	}
	int getsoldier(int r,int c){
		return get_soldier(whosee,r,c);
	}
	int getarmy(){
		assert(whosee>=0&&whosee<n);
		return army[whosee];
	}
	int getW(){
		return W;
	}
	int getH(){
		return H;
	}
	int getobservable(int r,int c){
		return cansee(whosee,r,c);
	}
};


struct rndbot:bot{
	std::mt19937 ran;
	rndbot(){
		ran=std::mt19937(time(0));
	}
	bool isstop(){
		return false;
	}
	int getrandomaction(){
		uniform_int_distribution<int> dist(0,ACTIONSZ-1);
		return dist(ran);
	}
	void startgame(){
		
	}
	pair<int,int> sol(int myindex,Game& g){
		int W=g.getW(),H=g.getH();
		while(1){
			int pos=getrandomaction(),row=pos/W,col=pos%W;
//			return make_pair(0,0);
			if(g.getmp(row,col)==myindex){
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
//				printf("<%d,%d>",pos,endpos);
				return make_pair(pos,endpos);
				break;
			}
		}
		assert(false);
	}	
	void putreward(double r){
		
	}
}O;

// 0: Observable
// 1: Map(mountain/ not mountain)
// 2: Soldiers
// 3: Armys' Owner
// 4: turn
// 5: Camp/not Camp
struct seqData{
	Tensor data;
	int action;
	seqData(){}
	seqData(Tensor data,int action):
		data(data),action(action){}
};
Tensor getnow(int myindex,Game& g){
	int W=g.getW(),H=g.getH();
	Tensor ret=torch::zeros({6,H,W});
	for(int i=0;i<H;++i)
		for(int j=0;j<W;++j){
			if(g.getobservable(i,j)){
				ret[0][i][j]=1.0;
				if((g.getmp(i,j)==myindex))ret[2][i][j]=g.getsoldier(i,j);
				ret[3][i][j]=(g.getmp(i,j)==myindex);
			}
			if(g.getmp(i,j)==TILE_MOUNTAIN||g.getmp(i,j)==TILE_FOG_OBSTACLE)
				ret[1][i][j]=1.0;
			if(g.getmp(i,j)==TILE_MOUNTAIN&&g.get_iscamp(i,j)==1)
				ret[5][i][j]=g.getsoldier(i,j);
		}
	ret[4][0][0]=(g.turn_count%50)/50.0;
	auto deal=[&](int id){
		double sumsold=0;
		for(int i=0;i<H;++i)
			for(int j=0;j<W;++j)
				sumsold+=ret[id][i][j].item().toFloat();
		if(sumsold<1)sumsold=1;
		for(int i=0;i<H;++i)
			for(int j=0;j<W;++j)
				ret[id][i][j]=ret[id][i][j].item().toFloat()/sumsold;	
	};
	deal(2);deal(4);
	return ret;
}
struct mybot:bot{
	const static int hidden_size=512;
	const static int batch_size=1e9;
	const static int ROUND=50;
	const static int conv1w=4,conv1h=4;
	const static int hiddensize=64;
	const static int conv2w=3,conv2h=3;
	const static int conv3w=3,conv3h=3;
	const static int inputsize=6;
	vector<seqData>epoch;
	vector<double>reward;
	struct ActorDRQN: nn::Cloneable<ActorDRQN>{
		nn::Conv2d conv1=nullptr;
		nn::Conv2d conv2=nullptr,conv3=nullptr;
		nn::BatchNorm2d bat1=nullptr;
		nn::BatchNorm1d bat2=nullptr;
		nn::LSTM lstm1=nullptr;
		nn::Linear linact1=nullptr,linact2=nullptr;
		nn::Linear lincri1=nullptr,lincri2=nullptr;
		ActorDRQN(int inputsz=4,int outputsz=ACTIONSZ){
			conv1=nn::Conv2d(nn::Conv2dOptions(inputsize,hiddensize,conv1w).padding(2));
			conv2=nn::Conv2d(nn::Conv2dOptions(hiddensize,hiddensize,conv2w));
			conv3=nn::Conv2d(nn::Conv2dOptions(hiddensize,hiddensize,conv3w));
			bat1=nn::BatchNorm2d(hiddensize);
			lstm1=nn::LSTM(nn::LSTMOptions((WIDTH-conv1w+1-conv2w+1-conv3w+1+4)*(HEIGHT-conv1h+1-conv2h+1-conv3h+1+4)*hiddensize,hidden_size).num_layers(2));
			bat2=nn::BatchNorm1d((WIDTH-conv1w+1-conv2w+1-conv3w+1)*(HEIGHT-conv1h+1-conv2h+1-conv3h+1)*hiddensize);
			linact1=nn::Linear(nn::LinearOptions(hidden_size,hidden_size));
			linact2=nn::Linear(nn::LinearOptions(hidden_size,ACTIONSZ));
			lincri1=nn::Linear(nn::LinearOptions(hidden_size,hidden_size));
			lincri2=nn::Linear(nn::LinearOptions(hidden_size,1));
			register_module("conv1",conv1);
			register_module("conv2",conv2);
			register_module("conv3",conv3);
			register_module("lstm1",lstm1);
			register_module("bat1",bat1);
			register_module("bat2",bat2);
			register_module("linact1",linact1);
			register_module("linact2",linact2);
			register_module("lincri1",lincri1);
			register_module("lincri2",lincri2);
		}
		tuple<Tensor,Tensor,tuple<Tensor,Tensor>> forward(Tensor x,tuple<Tensor,Tensor> hidden){
//			x.print();fflush(stdout);
			x=nn::functional::elu(conv1(x));
//			x.print();fflush(stdout);
			x=nn::functional::elu(conv2(x));
//			x.print();fflush(stdout);
			x=nn::functional::elu(conv3(x));
//			x.print();fflush(stdout);
			x=bat1(x);
			x=x.flatten(1,-1);
			x=x.unsqueeze(1);
//			x.print();fflush(stdout);
			auto lstmret=lstm1(x,hidden);
			x=std::get<0>(lstmret);
			hidden=std::get<1>(lstmret);
			x=x.squeeze();
//			x=bat2(x);
			Tensor act=nn::functional::elu(linact1(x));
//			cout<<act<<endl;
			act=linact2(act);
			act=softmax(act,0);
			act=torch::add(act,1e-9);
			Tensor cri=nn::functional::elu(lincri1(x));
			cri=lincri2(cri);
			return make_tuple(act,cri,hidden);
		}
		void reset(){		
			conv1=nn::Conv2d(nn::Conv2dOptions(inputsize,hiddensize,conv1w));
			conv2=nn::Conv2d(nn::Conv2dOptions(hiddensize,hiddensize,conv2w));
			conv3=nn::Conv2d(nn::Conv2dOptions(hiddensize,hiddensize,conv3w));
			bat1=nn::BatchNorm2d(hiddensize);
			lstm1=nn::LSTM(nn::LSTMOptions((WIDTH-conv1w+1-conv2w+1-conv3w+1)*(HEIGHT-conv1h+1-conv2h+1-conv3h+1)*hiddensize,hidden_size).num_layers(2));
			bat2=nn::BatchNorm1d((WIDTH-conv1w+1-conv2w+1-conv3w+1)*(HEIGHT-conv1h+1-conv2h+1-conv3h+1)*hiddensize);
			linact1=nn::Linear(nn::LinearOptions(hidden_size,hidden_size));
			linact2=nn::Linear(nn::LinearOptions(hidden_size,ACTIONSZ));
			lincri1=nn::Linear(nn::LinearOptions(hidden_size,hidden_size));
			lincri2=nn::Linear(nn::LinearOptions(hidden_size,1));
			register_module("conv1",conv1);
			register_module("conv2",conv2);
			register_module("conv3",conv3);
			register_module("lstm1",lstm1);
			register_module("bat1",bat1);
			register_module("bat2",bat2);
			register_module("linact1",linact1);
			register_module("linact2",linact2);
			register_module("lincri1",lincri1);
			register_module("lincri2",lincri2);
		}
		void print_grad(ofstream& o){
			for (const auto&p:parameters()){
				o<<p<<std::endl;
			}
		}
	};
	std::shared_ptr<ActorDRQN>Actor;
//	CriDRQN Cri;
//	std::shared_ptr<CriDRQN>yCri;
	std::tuple<Tensor, Tensor> hidden,hidden_cri,yhidden,yhidden_cri;
	double gamma,epsilon,alpha;
	int round_counter,max_step,nwstep,TAG;
	std::mt19937 ran;
	ofstream fout;
	mybot(){
		gamma=0.99;
		epsilon=0.9;
		alpha=0.9;
		round_counter=0;
		TAG=0;
		ran=std::mt19937(114514);
		Actor=std::shared_ptr<ActorDRQN>(new ActorDRQN);
//		yCri=std::dynamic_pointer_cast<CriDRQN>(Cri.clone());
		fout=ofstream(("bot.out"));
	}
	void startgame(){
		init_hidden();
		epoch.clear();
		reward.clear();
		nwstep=0;
	}
	bool isstop(){
		return nwstep>=max_step;
	}
	std::tuple<Tensor, Tensor> zerohidden(){
		return std::make_tuple(torch::zeros({2,1,hidden_size}),torch::zeros({2,1,hidden_size}));
	}
	void init_hidden(){
		hidden=zerohidden();
		hidden_cri=zerohidden();
		yhidden=zerohidden();
		yhidden_cri=zerohidden();
	}
	double cal_mean(Tensor t){
		double ret=0;
		t=t.flatten();
		for(int i=0;i<t.size(0);++i)
			ret+=t[i].item().toFloat();
		return ret/t.size(0);
	}
	double cal_sum(Tensor t){
		double ret=0;
		t=t.flatten();
		for(int i=0;i<t.size(0);++i)
			ret+=t[i].item().toFloat();
		return ret;
	}
	int get_action(Tensor policy){
		double t=-1e20;
		int ret=0;
		for(int i=0;i<policy.size(0);++i){ 
			if(t<policy[i].item().toFloat())
				t=policy[i].item().toFloat(),ret=i;
		}
		return ret;
	}
	
	void setepsilon(double x){
		epsilon=x;
	}
	void putreward(double r){
//		printf("[%.3lf]",r);
		reward.push_back(r);
	}
	double PRNG(){
		uniform_real_distribution<double> dist(0,1);
		return dist(ran);
	}
	int getrandomaction(){
		uniform_int_distribution<int> dist(0,ACTIONSZ-1);
		return dist(ran);
	}
	int get_prob_action(Tensor policy){
		double t=PRNG();
		int ret=policy.size(0)-1;
		for(int i=0;i<policy.size(0);++i){ 
			t-=policy[i].item().toFloat();
			if(t<=0){
				ret=i;
				break;
			}
		}
		return ret;
	}
	void load(){
		Actor.reset();
		Actor=std::shared_ptr<ActorDRQN>(new ActorDRQN);
		torch::load(Actor,"mybot.mdl");
	}
	void save(){
		torch::save(Actor,"mybot.mdl");
	}
	void train(){
		int W=WIDTH,H=HEIGHT;
		round_counter++;
		if(round_counter%ROUND==0){
//			yActor.reset();
//			yCri.reset();
//			yActor=std::dynamic_pointer_cast<ActorDRQN>(Actor.clone());
//			yCri=std::dynamic_pointer_cast<CriDRQN>(Cri.clone());
		}
        init_hidden();
        for(int i=0;i<reward.size();++i)
        	reward[i]/=MAXREWARD;
        int batch_size=1e9;//max((int)sqrt(epoch.size()),120);
        for(int batch=0;batch<epoch.size();batch+=batch_size){
        	cout<<batch<<endl;fflush(stdout);
        	int length=min(batch_size,(int)epoch.size()-batch);
        	int _length=min(batch_size+1,(int)epoch.size()-batch);
//        	printf("<%d,%d>",batch,epoch.size());fflush(stdout);
        	Tensor list=torch::zeros({_length,6,H,W});
	        for(int i=batch;i<batch+_length&&i<epoch.size();i++)
	        	list[i-batch]=epoch[i].data;
	        auto optimizer_actor=torch::optim::Adam(Actor->parameters(), torch::optim::AdamOptions(1e-3));
        	optimizer_actor.zero_grad();
        	
        	auto _output=Actor->forward(list,hidden);
        	auto output=std::get<0>(_output);
        	auto _output_cri=(std::get<1>(_output));
			auto output_cri=torch::zeros({length}),youtput_cri=torch::zeros({length});
			auto log_probs=torch::zeros({length}),entropy=torch::zeros({1});
//			puts("OK");fflush(stdout);
        	for(int i=0;i<length;++i){
        		if(i+batch==epoch.size()-1)
					youtput_cri[i]=reward[i];
				else youtput_cri[i]=reward[i]+gamma*_output_cri[i+1].item().toFloat();
				auto logs=torch::log(output[i]);
				log_probs[i]=logs[epoch[i+batch].action];
				entropy+=-(logs*output[i]).sum();
				output_cri[i]=_output_cri[i][0];
			}
//			puts("OK");fflush(stdout);
			auto advantage=youtput_cri-output_cri;
			auto adv=advantage.clone().detach();
			auto actor_loss=-(log_probs*adv).mean()-0.001*entropy;
			auto critic_loss=advantage.pow(2).mean();
			auto loss=actor_loss+critic_loss;
			cout<<"LOSS:"<<loss.item().toFloat()<<endl;
			loss.backward();
			optimizer_actor.step();
		}
	}
	int getvalidrandomaction(int myindex,Game& g){
		int W=g.getW(),H=g.getH();
		while(1){
			int pos=getrandomaction(),row=pos/W,col=pos%W;
			if(g.getmp(row,col)==myindex){
				int rnd=rand()%4;
				if(rnd==0&&col>0&&g.getmp(row,col-1)>=TILE_EMPTY){
					return pos*4;
				} else if(rnd==1&&col<W-1&&g.getmp(row,col+1)>=TILE_EMPTY){
					return pos*4+1;
				} else if(rnd==2&&row<H-1&&g.getmp(row+1,col)>=TILE_EMPTY){
					return pos*4+2;
				} else if(rnd==3&&row>0&&g.getmp(row-1,col)>=TILE_EMPTY){
					return pos*4+3;
				} else continue;
				break;
			}
		}
		assert(false);
	}
	pair<int,int> sol(int myindex,Game& g){
		nwstep++;
		Tensor t=getnow(myindex,g);
		int action=0,W=g.getW(),H=g.getH();
		if(PRNG()>epsilon){
			auto output_=Actor->forward(t.unsqueeze(0),hidden);
			Tensor policy=std::get<0>(output_);
//			cout<<policy<<endl;
//			cout<<std::get<1>(output_)<<endl;
			hidden=std::get<2>(output_);
			action=!TAG?get_action(policy):get_prob_action(policy);
		} else {
			auto output_=Actor->forward(t.unsqueeze(0),hidden);
			hidden=std::get<2>(output_);
//			action=ACTIONSZ-1;
			action=getvalidrandomaction(myindex,g);
		}
		int pos=action/4,endpos=dpos[action%4]+pos;
		epoch.push_back(seqData(t,action));
//		printf("{%d,%d}",pos,endpos);fflush(stdout);
		return make_pair(pos,endpos);
	}
	double getreward(){
		double sum=0;
		for(auto a:reward)
			sum+=a;
		return sum;
	}
}B[2];
const int mybot::hidden_size;
const int mybot::batch_size;
const int mybot::ROUND;
const int mybot::conv1w,mybot::conv1h;
const int mybot::hiddensize;
const int mybot::conv2w,mybot::conv2h;
const int mybot::conv3w,mybot::conv3h;
const int mybot::inputsize;

void MyMapinit(Game& g,int pos=WIDTH*HEIGHT-1){
	g.generalinit();
	int A[10][10]=
	{
{-1,-1,-1,-1,-2,-1,-1,-2,-2,-1},
{-1,-1,-2,-1,-1,-2,-1,-1,-1,-1},
{-1,-1,-1,-1,-1,-1,-2,-2,-1,-2},
{-2,-1,-2,-1,-1,-2,-1,-1,-1,-1},
{-1,-1,-1,-1,-1,-1,-2,-1,-1,-1},
{-1,-2,-1,-2,-1,-1,-2,-2,-2,-2},
{-1,-1,-1,-1,-1,-2,-1,-1,-1,-1},
{-2,-1,-1,-2,-1,-2,-1,-1,-1,-1},
{-1,-1,-1,-1,-2,-2,-1,-1,-1,-1},
{-1,-1,-1,-1,-1,-2,-1,-1,-1,-1}
	};
	int soldier[10][10]=
	{{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,40,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{40,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	};
	int iscamp[10][10]=
	{{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,1,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	};
	if(A[pos/WIDTH][pos%WIDTH]!=-1||pos==0||pos<15)pos=WIDTH*HEIGHT-1;
	g.generals[1]=66;
	g.generals[0]=pos;
	A[6][6]=1;
	A[pos/WIDTH][pos%WIDTH]=0;
	for(int i=0;i<HEIGHT;++i)
		for(int j=0;j<WIDTH;++j)
			g.A[i][j]=A[i][j],g.soldier[i][j]=soldier[i][j],g.iscamp[i][j]=iscamp[i][j];
}

void make_bot_walk_correctly(Game& g,mybot& b){
	b.max_step=8;
	int test_round=10;
	double epsilon=1;
	int R=(WIDTH*HEIGHT)-1;
	printf("[%d]",R);
	for(int i=0;b.max_step<=15;++i){
		b.TAG=1;
		b.setepsilon(epsilon=(epsilon<=0.2?1:epsilon-0.1));
		b.startgame();
		MyMapinit(g,R);
		g.playerinit(0,&b);
		g.playerinit(1,&O);
		int winner=g.start();
		printf("[%.2lf]",b.getreward());
//		printf("[%d,%d]\n",winner,++TURN);
		fflush(stdout);
		b.train();
		if(i%test_round==0){
			b.TAG=1;
			b.setepsilon(0);
			b.startgame();
			MyMapinit(g,R);
			g.playerinit(0,&b);
			g.playerinit(1,&O);
			int winner=g.start();
			if(winner==0||(winner==-1&&b.getreward()>=(-3)))
				b.max_step++
				//,R=abs(rand())%(WIDTH*HEIGHT)
				,epsilon=0.8,printf("[Yes,%d,%d]\n",i,b.max_step);
			else printf("[No,%d,%d]\n",i,b.max_step);
		}
	}
}
int main(){
	srand(time(0));
	Game g(WIDTH,HEIGHT,2);
	int TURN=0;
//	B[0].load();
	make_bot_walk_correctly(g,B[0]); 
//	B[0].save();
	B[0].TAG=1;
	double epsilon=0.9;
	while(1){
		B[0].TAG=1;
//		B[0].setepsilon(0); 
		B[0].setepsilon(epsilon=epsilon<=0.01?0.9:epsilon-0.01);
		B[0].startgame();
		B[0].max_step=3e3;
	//	B[1].startgame();
	//	B[1].setepsilon(B[1].epsilon-0.01);
		MyMapinit(g);
		g.playerinit(0,&B[0]);
		g.playerinit(1,&O);
		int winner=g.start();
		printf("[%d,%d]\n",winner,++TURN);
		fflush(stdout);
		B[0].train();
		if(TURN%20==0){
			B[0].TAG=1;
			B[0].save();
			B[0].load();
			int S[3]={0};
			for(int i=0;i<10;++i){
				B[0].setepsilon(0);
				B[0].startgame();
				B[0].max_step=3e3;
			//	B[1].startgame();
			//	B[1].setepsilon(B[1].epsilon-0.01);
				MyMapinit(g);
				g.playerinit(0,&B[0]);
				g.playerinit(1,&O);
				int winner=g.start();
				if(winner==-1)S[2]++;
				else S[winner]++;
			}
			printf("test:%d:%d(%d)\n",S[0],S[1],S[2]);
			B[0].fout<<"test:"<<S[0]<<":"<<S[1]<<">"<<S[2]<<endl;
		}
	///	B[1].train(winner==0?100:-100);
//		printf("[train complete.(%d)]\n",++TURN);
//		if(TURN%10==0){ 
//			torch::save(std::make_shared<Net>(),"model.pt");
//		}
//		if(TURN%20==0){
//			B[1]=B[0];
//		}
	}
}
