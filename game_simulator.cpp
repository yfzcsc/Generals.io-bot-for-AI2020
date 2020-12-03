#include<bits/stdc++.h>
using namespace std;
const int TILE_EMPTY=-1; 
const int TILE_MOUNTAIN=-2;
const int TILE_FOG=-3;
const int TILE_FOG_OBSTACLE=-4;
const int N=500;
class Game{
public:
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
	function<pair<int,int>(int,Game&)>v[N];
	int cansee(int id,int r,int c){
		if(id==-1)return true;
		return (r>0&&A[r-1][c]==id)||(c>0&&A[r][c-1]==id)||(r<W-1&&A[r+1][c]==id)||(r<H-1&&A[r][c+1]==id)||A[r][c]==id;
	}
	int checkin(int r,int c){
		return r<0||c<0||r>=W||c>=H;
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
	bool check_move(int id,int bg,int ed){
		if(bg==ed||checkin(bg/W,bg%W)||checkin(ed/W,ed%W))return false;
		if(get_mp(id,bg)!=id||get_soldier(id,bg)<=1)return false;
		if(A[ed/W][ed%W]==TILE_MOUNTAIN&&iscamp[ed/W][ed%W]!=1)return false;
		if(abs(bg-ed)!=1&&abs(bg-ed)!=W)return false;
		if(abs(bg-ed)==1&&min(bg,ed)%W==W-1)return false;
		return true;
	}
	void act(int id,pair<int,int> p){
		int bg=p.first;
		int ed=p.second;
		int army=soldier[bg/W][bg%W]-1;
		soldier[bg/W][bg%W]=1;
		if(A[ed/W][ed%W]==id)soldier[ed/W][ed%W]+=army;
		else if(soldier[ed/W][ed%W]<army)A[ed/W][ed%W]=id,soldier[ed/W][ed%W]=army-soldier[ed/W][ed%W];
		else soldier[ed/W][ed%W]-=army;
	}
	void do_lost(int winner,int loser){
		for(int i=0;i<W;++i)
			for(int j=0;j<H;++j)
				if(A[i][j]==loser)
					A[i][j]=winner;
		army[winner]+=army[loser];
	}
	void doswitch(int player){
		whosee=player;
	}
	
public:
	void print_mp(){
		for(int i=0;i<W*H;++i)printf("%d%c",get_mp(-1,i),i%W==W-1?'\n':' ');puts("\n>>>>>>>>>>");
		for(int i=0;i<W*H;++i)printf("%d%c",get_soldier(-1,i),i%W==W-1?'\n':' ');puts("\n>>>>>>>>>>");
	}
	int start(){
		static bool vis[N],lost[N];
		static pair<int,int> pos[N];
		static int Q[N],ls[N],rest;
		rest=n;
		for(int i=0;i<n;++i)assert(tg[i]);
		for(int turn=1;;turn++){
			for(int i=0;i<n;++i)vis[i]=0;
			for(int i=0;i<n;++i)if(!lost[i]){
				doswitch(i);
				pair<int,int>p=v[i](i,*this);
				if(check_move(i,p.first,p.second))
					pos[i]=p;
				else vis[i]=true;
				doswitch(-1);
			} else vis[i]=true;
			
			for(int i=0;i<n;++i)if(!vis[i]){
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
			}
		//	for(int i=0;i<n;++i)if(!vis[i]){
		//		if(check_move(i,pos[i].first,pos[i].second))
		//			act(i,pos[i]);
		//	}
			// lose
			for(int i=0;i<n;++i)if(!lost[i])
				if(A[generals[i]/W][generals[i]%W]!=i){
					rest--;
					lost[i]=true;
					do_lost(A[generals[i]/W][generals[i]%W],i);
				}
			if(rest==1){
				printf("Game end at turn:%d\n",turn);
				print_mp();
				for(int i=0;i<n;++i)if(!lost[i])
					return i;
			}
			// soldier increase
			if(turn%TURN_MAP==0){
				for(int i=0;i<W;++i)
					for(int j=0;j<H;++j)
						if(A[i][j]>=0)soldier[i][j]++,army[A[i][j]]++;
			} 
			if(turn%TURN_KING==0){
				for(int i=0;i<W;++i)
					for(int j=0;j<H;++j)
						if(A[i][j]>=0&&iscamp[i][j])soldier[i][j]++,army[A[i][j]]++;
				for(int i=0;i<n;++i)soldier[generals[i]/W][generals[i]%W]++,army[i]++;
			}
		}
	}
	void generalinit(){
		for(int i=0;i<W;++i)
			for(int j=0;j<H;++j)
				A[i][j]=soldier[i][j]=iscamp[i][j]=0;
		for(int i=0;i<n;++i)tg[i]=army[i]=0,generals[i]=-1;
		whosee=-1;
	}
	void rndinit(int mountains,int camps){
		assert(mountains+camps+n<=W*H);
		generalinit();
		for(int i=0;i<W;++i)
			for(int j=0;j<H;++j)
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
	void playerinit(int id,function<pair<int,int>(int,Game&)> func){
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
};
pair<int,int> rndbot(int myindex,Game& g){
	int W=g.getW(),H=g.getH();	
	static int x=134985,y=29,z=0;
	x=1ll*x*y%19260817;
	int x0=1ll*x*x%19260817;
	
	if(x%8<=2)z=x0%100-1;
	else if(x%8<=4)z=x0%100+1;
	else if(x%8<=6)z=x0%100-10;
	else z=x0%100+10;
//	printf("[%d,%d,%d]\n",myindex,x0%100,z);
	return make_pair(x0%100,z);
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
		//	printf("<%d,%d>\n",pos,endpos);
			return make_pair(pos,endpos);
			break;
		}
	}
	assert(false);
}
pair<int,int> nomovebot(int myindex,Game& g){
	return make_pair(0,0);
	assert(false);
}
void MyMapinit(Game& g){
	g.generalinit();
	int A[10][10]=
	{{0,-1,-2,-2,-2,-2,-1,-1,-2,-2},
	{-1,-1,-1,-2,-2,-1,-1,-1,-2,-2},
	{-2,-1,-1,-2,-2,-1,-2,-1,-1,-1},
	{-2,-2,-1,-2,-2,-2,-1,-1,-1,-2},
	{-2,-2,-1,-2,-2,-2,-1,-1,-2,-2},
	{-2,-1,-1,-2,-2,-2,-1,-2,-2,-2},
	{-1,-1,-2,-1,-2,-2,-1,-1,-1,-2},
	{-1,-1,-1,-2,-1,-2,-1,-1,-1,-1},
	{-2,-2,-1,-1,-2,-2,-1,-1,-1,-1},
	{-2,-2,-1,-2,-2,-2,-2,-1,-1,1}
	};
	int soldier[10][10]=
	{{1,0,40,40,40,0,0,0,40,40},
	{0,0,0,40,0,0,0,0,40,40},
	{40,0,0,0,0,0,0,0,0,0},
	{40,40,0,40,40,0,0,0,0,0},
	{40,0,0,40,40,40,0,0,0,40},
	{0,0,0,0,40,40,0,0,40,40},
	{0,0,0,0,40,40,0,0,0,40},
	{0,0,0,0,0,0,0,0,0,0},
	{40,40,0,0,0,40,0,0,0,0},
	{40,40,0,0,40,40,40,0,0,1},
	};
	int iscamp[10][10]=
	{{0,0,1,1,1,0,0,0,1,1},
	{0,0,0,1,0,0,0,0,1,1},
	{1,0,0,0,0,0,0,0,0,0},
	{1,1,0,1,1,0,0,0,0,0},
	{1,0,0,1,1,1,0,0,0,1},
	{0,0,0,0,1,1,0,0,1,1},
	{0,0,0,0,1,1,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0},
	{1,1,0,0,0,1,0,0,0,0},
	{1,1,0,0,1,1,1,0,0,0},
	};
	for(int i=0;i<10;++i)
		for(int j=0;j<10;++j)
			g.A[i][j]=A[i][j],g.soldier[i][j]=soldier[i][j],g.iscamp[i][j]=iscamp[i][j];
	g.generals[0]=0;
	g.generals[1]=99;
}


int main(){
	Game g(10,10,2);
	MyMapinit(g);
//	g.rndinit(30,30);
	g.playerinit(0,rndbot);
	g.playerinit(1,rndbot);
	g.start();
}
