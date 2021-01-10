#include<torch/torch.h>
#include<bits/stdc++.h>
using namespace torch;
using namespace std;
const int TILE_EMPTY=-1; 
const int TILE_MOUNTAIN=-2;
const int TILE_FOG=-3;
const int TILE_FOG_OBSTACLE=-4;
const int N=500,WIDTH=3,HEIGHT=3,ACTIONSZ=4*WIDTH*HEIGHT;
class Game;
struct bot{
	void virtual putreward(int r){}
	pair<int,int> virtual sol(int myindex,Game& g){} 
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
	int cansee(int id,int r,int c){
		if(id==-1)return true;
		return (r>0&&A[r-1][c]==id)||(c>0&&A[r][c-1]==id)||(r<H-1&&A[r+1][c]==id)||(c<W-1&&A[r][c+1]==id)||A[r][c]==id;
	}
	int checkin(int r,int c){
		return r<0||c<0||r>=H||c>=W;
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
		if(get_mp(id,bg)!=id||get_soldier(id,bg)<=1)return false;
		if(A[ed/W][ed%W]==TILE_MOUNTAIN&&iscamp[ed/W][ed%W]!=1)return false;
		if(abs(bg-ed)!=1&&abs(bg-ed)!=W)return false;
		if(abs(bg-ed)==1&&min(bg,ed)%W==W-1)return false;
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
		for(int i=0;i<n;++i)assert(tg[i]);
		for(int i=0;i<n;++i)army[i]=1,lost[i]=0;
		for(int turn=1;;turn++){
//			print_mp();
			for(int i=0;i<n;++i)vis[i]=0;
			for(int i=0;i<n;++i)if(!lost[i]){
				doswitch(i);
				pair<int,int>p=v[i]->sol(i,*this);
				if(check_move(i,p.first,p.second))
					pos[i]=p;
				else vis[i]=true;
				doswitch(-1);
			} else vis[i]=true;
			for(int i=0;i<n;++i){
				v[i]->putreward(-1+(vis[i]?-100:0));
			}
			/*for(int i=0;i<n;++i)if(!vis[i]){
				int flg=0;
				for(int j=0;j<n;++j)if(pos[i].second==pos[j].first&&!vis[j])
					flg=1;
				if(!flg){
					int qr=0;
					Q[qr++]=i;
					for(int ql=0;ql<qr;ql++){
						int nowact=Q[ql++];
						act(nowact,pos[nowact]);
						for(int j=0;j<n;++j)if(pos[j].second==pos[nowact].first&&!vis[j])
							Q[qr++]=j;
					}
					for(int ql=0;ql<qr;ql++)vis[Q[ql]]=true;
				}
			}
			//cycle
			for(int i=0;i<n;++i)if(!vis[i]){
				ls[i]=soldier[pos[i].first/W][pos[i].first%W]-2;
				soldier[pos[i].first/W][pos[i].first%W]=0;
			}
			for(int i=0;i<n;++i)if(!vis[i]&&soldier[pos[i].first/W][pos[i].first%W]>0){
				A[pos[i].second/W][pos[i].second%W]=i;
				soldier[pos[i].second/W][pos[i].second%W]=ls[i];
			}*/
			for(int i=0;i<n;++i)if(!vis[i]){
				if(check_move(i,pos[i].first,pos[i].second))
					act(i,pos[i]);
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
//				print_mp();
				for(int i=0;i<n;++i)if(!lost[i])
					return i;
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
	pair<int,int> sol(int myindex,Game& g){
		int W=g.getW(),H=g.getH();
		while(1){
			int pos=rand()%(W*H),row=pos/W,col=pos%W;
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
				return make_pair(pos,endpos);
				break;
			}
		}
		assert(false);
	}	
	void putreward(int r){
		
	}
}O;

struct NetDRQN: nn::Module{
	nn::Conv2d conv1,conv3;
//	nn::LSTM lstm;
//	nn::Linear lin1;
	NetDRQN(int inputsz=4,int outputsz=ACTIONSZ)
	:conv1(nn::Conv2dOptions(inputsz,512,{2,2})),
//	conv2(nn::Conv2dOptions(64,32,2)),
	conv3(nn::Conv2dOptions(512,ACTIONSZ,{2,2})){
		register_module("conv1",conv1);
//		register_module("conv2",conv2);
		register_module("conv3",conv3);
	}
	Tensor forward(Tensor x){
		x=tanh(conv1(x));
//		x=relu(conv2(x));
		x=conv3(x);
//		x.print();
		x=x.squeeze();
		return x;
	}
};
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
				ret[0][i][j]=1;
				ret[2][i][j]=g.getsoldier(i,j);
				sumsold+=ret[2][i][j].item().toFloat()*ret[2][i][j].item().toFloat();
				ret[3][i][j]=max(g.getmp(i,j),0);
			}
			if(g.getmp(i,j)==TILE_MOUNTAIN||g.getmp(i,j)==TILE_FOG_OBSTACLE)
				ret[1][i][j]=1;
		}
	sumsold=sqrt(sumsold);
	if(sumsold<1)sumsold=1;
	for(int i=0;i<H;++i)
		for(int j=0;j<W;++j)
			ret[2][i][j]/=sumsold;
	return ret;
}
struct mybot:bot{
	vector<seqData>epoch;
	vector<int>reward;
	NetDRQN A;
	double gamma,epsilon,alpha;
	mybot(){
		gamma=0.9;
		epsilon=0.8;
		alpha=0.9;
	}
	void startgame(){
		epoch.clear();
		reward.clear();
	}
	void train(int score){
		int W=WIDTH,H=HEIGHT;
//		printf("[epoch%d]",epoch.size());fflush(stdout);
		auto optimizer=torch::optim::Adam(A.parameters(), torch::optim::AdamOptions(0.1));
        optimizer.zero_grad();
        
        Tensor list=torch::zeros({epoch.size(),4,H,W});
        for(int i=0;i<epoch.size();++i)list[i]=epoch[i].data;
        
		auto Q=A.forward(list);
		auto Qtarget=Q.clone().detach();
		for(int i=0;i<epoch.size()-1;++i){
			double mx=0;
			for(int j=0;j<ACTIONSZ;++j)if(mx<Q[i+1][j].item().toFloat())mx=Q[i+1][j].item().toFloat();
			Qtarget[i][epoch[i].action]=gamma*mx+reward[i];
//			printf("<%d>",reward[i]);
		}
		Qtarget[epoch.size()-1][epoch.back().action]=score;
        auto loss = torch::mse_loss(Q,Qtarget);
        cout<<loss<<endl;
//        cout<<Qtarget<<endl;
        
        loss.backward();
        optimizer.step();
//        Q=A.forward(list);
//        cout<<Q<<endl;
//        exit(0);
	}
	void setepsilon(double x){
		epsilon=max(x,0.01);
	}
	void putreward(int r){
		reward.push_back(r);
	}
	pair<int,int> sol(int myindex,Game& g){
		Tensor t=getnow(myindex,g);
		int action=0,W=g.getW(),H=g.getH();
		if(rand()/(double)RAND_MAX>epsilon){
			auto output=A.forward(t.unsqueeze(0));
//			output.print();
//			for(int j=0;j<ACTIONSZ;++j)printf("%.3lf,",(double)output[j].item().toFloat());puts("");
			double mx=-1e20;
			for(int j=0;j<ACTIONSZ;++j)if(mx<output[j].item().toFloat())mx=output[j].item().toFloat(),action=j;
//			printf("<%d>",action);
		} else {
	//		auto output=B.A.forward(t.unsqueeze(0));
	//		output.print();
			action=rand()%ACTIONSZ;
		}
		int pos=action/4,endpos;
		if(action%4==0)endpos=pos-1;
		else if(action%4==1)endpos=pos+1;
		else if(action%4==2)endpos=pos-W;
		else endpos=pos+W;
		epoch.push_back(seqData(t,action));
//		printf("{%d, %d}",pos,endpos);
		return make_pair(pos,endpos);
	}	
}B[2];

void MyMapinit(Game& g){
	g.generalinit();
	int A[5][5]=
	{{0,-1,-1,-1,-1},
	{-1,-1,-1,-1,-1},
	{-1,-1,-1,-1,-1},
	{-1,-1,-1,-1,-1},
	{-1,-1,-1,-1,1}
	};
	int soldier[5][5]=
	{{0,0,0,0,0},
	{0,0,0,0,0},
	{0,0,0,0,0},
	{0,0,0,0,0},
	{0,0,0,0,0}
	};
	int iscamp[5][5]=
	{{0,0,0,0,0},
	{0,0,0,0,0},
	{0,0,0,0,0},
	{0,0,0,0,0},
	{0,0,0,0,0}
	};
	for(int i=0;i<HEIGHT;++i)
		for(int j=0;j<WIDTH;++j)
			g.A[i][j]=A[i][j],g.soldier[i][j]=soldier[i][j],g.iscamp[i][j]=iscamp[i][j];
	g.generals[0]=0;
	g.generals[1]=WIDTH*HEIGHT-1;
}


int main(){
	srand(time(0));
	Game g(WIDTH,HEIGHT,2);
	int TURN=0;
	ofstream out("f.txt");
	while(1){
		B[0].setepsilon(B[0].epsilon-0.001);
		B[0].startgame();
	//	B[1].startgame();
	//	B[1].setepsilon(B[1].epsilon-0.01);
		MyMapinit(g);
		g.playerinit(0,&B[0]);
		g.playerinit(1,&O);
		++TURN;
		int winner=g.start(TURN%100==0);
		B[0].train(winner==0?1000:-1000);
		S[winner]++;
		out<<S[0]/TURN<<",";
		if(TURN%100==0,1)printf("[%d,%d(%.5lf)]",winner,++TURN,S[0]/TURN),fflush(stdout);
	///	B[1].train(winner==0?100:-100);
//		printf("[train complete.(%d)]\n",++TURN);
//		if(TURN%10==0){
//			torch::save(std::make_shared<Net>(),"model.pt");
//		}
//		if(TURN%20==0){
//			B[1]=B[0];
//		}
		if(TURN>=2000)break;
	}
}
