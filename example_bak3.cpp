#include<torch/torch.h>
#include<bits/stdc++.h>
using namespace torch;
using namespace std;
const int TILE_EMPTY=-1; 
const int TILE_MOUNTAIN=-2;
const int TILE_FOG=-3;
const int TILE_FOG_OBSTACLE=-4;
const int N=500,WIDTH=4,HEIGHT=4,ACTIONSZ=4*WIDTH*HEIGHT;
const int dx[4]={0,0,1,-1},dy[4]={-1,1,0,0},dpos[4]={-1,1,WIDTH,-WIDTH};
const double MAXREWARD=100.0;
class Game;
struct bot{
	bool virtual isstop(){}
	void virtual startgame(){}
	void virtual putreward(int r){} // called each round
	pair<int,int> virtual sol(int myindex,Game& g){} // called each round 
};

class Game{
public:
	int TURN;
	int A[N][N],soldier[N][N],iscamp[N][N],generals[N],W,H,n,tg[N],army[N];
	Game(int W,int H,int n):W(W),H(H),n(n){}
private:
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
	void act(int id,pair<int,int> p){
		int bg=p.first;
		int ed=p.second;
		int armys=soldier[bg/W][bg%W]-1;
		soldier[bg/W][bg%W]=1;
		if(A[ed/W][ed%W]==id)soldier[ed/W][ed%W]+=armys;
		else if(soldier[ed/W][ed%W]<armys){
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
		rest=n;
		whosee=-1;
		for(int i=0;i<n;++i)assert(tg[i]),assert(generals[i]>=0);
		for(int i=0;i<n;++i)army[i]=1,lost[i]=0;
		for(int i=0;i<n;++i)soldier[generals[i]/W][generals[i]%W]=1;
		for(int i=0;i<n;++i)v[i]->startgame();
		for(int turn=1;;turn++){
			for(int i=0;i<n;++i)vis[i]=0;
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
					act(r[i],pos[r[i]]);
			}
			// lose
			for(int i=0;i<n;++i)if(!lost[i])
				if(A[generals[i]/W][generals[i]%W]!=i){
					rest--;
					lost[i]=true;
					do_lost(A[generals[i]/W][generals[i]%W],i);
				}
			
			if(rest==1){
				TURN=turn;
				printf("Game end at turn:%d,",turn);
				for(int i=0;i<n;++i){
					v[i]->putreward(lost[i]?-100:100);
				}
//				print_mp();
				for(int i=0;i<n;++i)if(!lost[i])
					return i;
			}
			for(int i=0;i<n;++i){
				v[i]->putreward((vis[i]?-10:0));
			}
			for(int i=0;i<n;++i){
				TURN=turn;
				if(v[i]->isstop())
					return -1;
			}
			// soldier increase
			if(turn%TURN_MAP==0){
				for(int i=0;i<H;++i)
					for(int j=0;j<W;++j)
						if(A[i][j]>=0)soldier[i][j]++,army[A[i][j]]++;
			} 
			if(turn%TURN_KING==0){
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
	void putreward(int r){
		
	}
}O;

// 0: Observable
// 1: Map(mountain/ not mountain)
// 2: Soldiers
// 3: Armys' Owner
struct seqData{
	Tensor data;
	int action;
	seqData(){}
	seqData(Tensor data,int action):
		data(data),action(action){}
};
Tensor getnow(int myindex,Game& g){
	int W=g.getW(),H=g.getH();
	Tensor ret=torch::zeros({4,H,W});
	double sumsold=0;
	for(int i=0;i<H;++i)
		for(int j=0;j<W;++j){
			if(g.getobservable(i,j)){
				ret[0][i][j]=1.0/(W*H);
				ret[2][i][j]=g.getsoldier(i,j);
				sumsold+=ret[2][i][j].item().toFloat();
				ret[3][i][j]=(g.getmp(i,j)==myindex)/(W*H);
			}
			if(g.getmp(i,j)==TILE_MOUNTAIN||g.getmp(i,j)==TILE_FOG_OBSTACLE)
				ret[1][i][j]=1.0/(W*H);
		}
	if(sumsold<1)sumsold=1;
	for(int i=0;i<H;++i)
		for(int j=0;j<W;++j)
			ret[2][i][j]/=sumsold;
	return ret;
}
struct mybot:bot{
	const static int hidden_size=256;
	const static int batch_size=1e9;
	const static int ROUND=10;
	vector<seqData>epoch;
	vector<double>reward;
	struct ActorDRQN: nn::Cloneable<ActorDRQN>{
		nn::Conv2d conv1=nullptr;
		nn::LSTM lstm1=nullptr;
		nn::Linear linact1=nullptr,linact2=nullptr;
		ActorDRQN(int inputsz=4,int outputsz=ACTIONSZ){
			conv1=nn::Conv2d(nn::Conv2dOptions(4,32,{2,2}));
			lstm1=nn::LSTM(nn::LSTMOptions((WIDTH-2+1)*(HEIGHT-2+1)*32,hidden_size));
			linact1=nn::Linear(nn::LinearOptions(hidden_size,256));
			linact2=nn::Linear(nn::LinearOptions(256,ACTIONSZ));
			register_module("conv1",conv1);
			register_module("lstm1",lstm1);
			register_module("linact1",linact1);
			register_module("linact2",linact2);
		}
		tuple<Tensor,tuple<Tensor,Tensor>> forward(Tensor x,tuple<Tensor,Tensor> hidden){
			x=tanh(conv1(x));
			x=x.flatten(1,-1);
			x=x.unsqueeze(1);
//			x.print();fflush(stdout);
			auto lstmret=lstm1(x,hidden);
			x=std::get<0>(lstmret);
			hidden=std::get<1>(lstmret);
			x=x.squeeze();
			Tensor act=relu(linact1(x));
//			cout<<act<<endl;
			act=linact2(act);
			return make_tuple(act,hidden);
		}
		void reset(){		
			conv1=nn::Conv2d(nn::Conv2dOptions(4,10,{2,2}));
			lstm1=nn::LSTM(nn::LSTMOptions((WIDTH-2+1)*(HEIGHT-2+1)*10,hidden_size));
			linact1=nn::Linear(nn::LinearOptions(hidden_size,128));
			linact2=nn::Linear(nn::LinearOptions(128,ACTIONSZ));
			register_module("conv1",conv1);
			register_module("lstm1",lstm1);
			register_module("linact1",linact1);
			register_module("linact2",linact2);
		}
		void print_grad(ofstream& o){
			for (const auto&p:parameters()){
				o<<p<<std::endl;
			}
		}
	};
	ActorDRQN Actor;
	std::shared_ptr<ActorDRQN>yActor;
	std::tuple<Tensor, Tensor> hidden,hidden_cri,yhidden,yhidden_cri;
	double gamma,epsilon,alpha;
	int round_counter,max_step,nwstep;
	std::mt19937 ran;
	ofstream fout;
	mybot(){
		gamma=0.99;
		epsilon=0.9;
		alpha=0.9;
		round_counter=0;
		ran=std::mt19937(114514);
		yActor=std::dynamic_pointer_cast<ActorDRQN>(Actor.clone());
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
		return std::make_tuple(torch::zeros({1,1,hidden_size}),torch::zeros({1,1,hidden_size}));
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
	void train(){
		int W=WIDTH,H=HEIGHT;
		round_counter++;
		if(round_counter%ROUND==0){
			yActor.reset();
			yActor=std::dynamic_pointer_cast<ActorDRQN>(Actor.clone());
		}
        init_hidden();
        for(int i=0;i<reward.size();++i)
        	reward[i]/=MAXREWARD;
        for(int batch=0;batch<epoch.size();batch+=batch_size){
        	int length=min(batch_size+1,(int)epoch.size()-batch);
//        	printf("<%d,%d>",batch,epoch.size());fflush(stdout);
        	Tensor list=torch::zeros({length,4,H,W});
	        for(int i=batch;i<epoch.size()&&i<=batch+batch_size;i++)
	        	list[i-batch]=epoch[i].data;
	        auto optimizer_actor=torch::optim::Adam(Actor.parameters(), torch::optim::AdamOptions(1e-4));
        	optimizer_actor.zero_grad();
        	
        	auto output=std::get<0>(Actor.forward(list,hidden));
        	auto youtput=std::get<0>(yActor->forward(list,yhidden));
        	
        	auto y=torch::zeros({length});
        	auto _output=torch::zeros({length});
        	for(int i=0;i<length;++i){
        		if(i+batch==epoch.size()-1)
        			y[i]=reward[i];
        		else y[i]=reward[i]+gamma*youtput[i+1][epoch[i+batch].action].item().toFloat();
        		_output[i]=output[i][epoch[i+batch].action];
			}
//			cout<<"SIZE:"<<length<<" "<<_output_cri<<endl<<youtput_cri<<endl;
//			for(int i=0;i<output_cri.size(0);++i)
//				assert(output_cri[i].item().toFloat()==_output_cri[i].item().toFloat());
			auto advantage=y-_output;
			auto loss_actor=advantage.pow(2).mean();
			loss_actor.backward();
			optimizer_actor.step();
		}
	}
	void setepsilon(double x){
		epsilon=x;
	}
	void putreward(int r){
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
	int getvalidrandomaction(int myindex,Game& g){
		int W=g.getW(),H=g.getH();
		while(1){
			int pos=getrandomaction(),row=pos/W,col=pos%W;
			if(g.getmp(row,col)==myindex){
				int rnd=rand()%4;
				if(rnd==0&&col>0){
					return pos*4;
				} else if(rnd==1&&col<W-1){
					return pos*4+1;
				} else if(rnd==2&&row<H-1){
					return pos*4+2;
				} else if(rnd==3&&row>0){
					return pos*4+3;
				} else continue;
				break;
			}
		}
		assert(false);
	}	
	pair<int,int> real_sol(int myindex,Game& g){
		nwstep++;
		Tensor t=getnow(myindex,g);
		int action=0,W=g.getW(),H=g.getH();
		
		auto output_=Actor.forward(t.unsqueeze(0),hidden);
		Tensor policy=std::get<0>(output_);
//			cout<<policy<<endl;
		hidden=std::get<1>(output_);
		action=get_action(policy);
		int pos=action/4,endpos=dpos[action%4]+pos;
		epoch.push_back(seqData(t,action));
//		printf("{%d,%d}",pos,endpos);
		return make_pair(pos,endpos);
	}
	
	pair<int,int> sol(int myindex,Game& g){
		nwstep++;
		Tensor t=getnow(myindex,g);
		int action=0,W=g.getW(),H=g.getH();
		if(PRNG()>epsilon){
			auto output_=Actor.forward(t.unsqueeze(0),hidden);
			Tensor policy=std::get<0>(output_);
//			cout<<policy<<endl;
			hidden=std::get<1>(output_);
			action=get_action(policy);
		} else {
			auto output_=Actor.forward(t.unsqueeze(0),hidden);
			hidden=std::get<1>(output_);
			action=PRNG()>0.2?getvalidrandomaction(myindex,g):getrandomaction();
		}
		int pos=action/4,endpos=dpos[action%4]+pos;
		epoch.push_back(seqData(t,action));
		printf("{%d,%d}",pos,endpos);
		return make_pair(pos,endpos);
	}
	int getreward(){
		int sum=0;
		for(auto a:reward)
			sum+=a;
		return sum;
	}
}B[2];

void MyMapinit(Game& g){
	g.generalinit();
	int A[10][10]=
	{
{-1,-1,-1,-1,-2,-1,-1,-2,-2,-1},
{-1,-1,-2,-1,-1,-2,-1,-1,-1,-2},
{-1,-1,-1,-1,-1,-1,-2,-2,-1,-2},
{-2,-1,-2,-1,-1,-2,-1,-1,-1,-1},
{-1,-2,-1,-1,-1,-1,-2,-1,-1,-1},
{-1,-2,-1,-2,-1,-1,-1,-1,-1,-1},
{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{-2,-1,-1,-2,-1,-1,-1,-1,-1,-1},
{-1,-1,-1,-1,-2,-2,-1,-1,-1,-1},
{-1,-1,-1,-1,-1,-1,-2,-1,-1,-1}
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
	g.generals[1]=0;
	g.generals[0]=HEIGHT*WIDTH-1;
	A[0][0]=1;
	A[HEIGHT-1][WIDTH-1]=0;
	for(int i=0;i<HEIGHT;++i)
		for(int j=0;j<WIDTH;++j)
			g.A[i][j]=A[i][j],g.soldier[i][j]=soldier[i][j],g.iscamp[i][j]=iscamp[i][j];
}

void make_bot_walk_correctly(Game& g,mybot& b){
	b.max_step=2;
	int test_round=5;
	double epsilon=0.8;
	for(int i=0;b.max_step<=30;++i){
		b.setepsilon(epsilon=max(0.01,epsilon-0.0001));
		b.startgame();
		MyMapinit(g);
		g.playerinit(0,&b);
		g.playerinit(1,&O);
		int winner=g.start();
		printf("[%d]",b.getreward());
//		printf("[%d,%d]\n",winner,++TURN);
		fflush(stdout);
		b.train();
		if(i%test_round==0){
			b.setepsilon(0);
			b.startgame();
			MyMapinit(g);
			g.playerinit(0,&b);
			g.playerinit(1,&O);
			int winner=g.start();
			if(winner==0||(winner==-1&&b.getreward()>=(-b.max_step-5)))
				b.max_step++,epsilon=0.8,printf("[Yes,%d,%d]\n",i,b.max_step);
			else printf("[No,%d,%d]\n",i,b.max_step);
		}
	}
}
int main(){
	srand(time(0));
	Game g(WIDTH,HEIGHT,2);
	int TURN=0;
	make_bot_walk_correctly(g,B[0]);
	while(1){
		B[0].setepsilon(max(0.01,B[0].epsilon-0.001));
		B[0].startgame();
		B[0].max_step=10000;
	//	B[1].startgame();
	//	B[1].setepsilon(B[1].epsilon-0.01);
		MyMapinit(g);
		g.playerinit(0,&B[0]);
		g.playerinit(1,&O);
		int winner=g.start();
		printf("[%d,%d]\n",winner,++TURN);
		fflush(stdout);
		B[0].train();
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
