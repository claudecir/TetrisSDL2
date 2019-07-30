// IO includes
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

// Strings e char
#include <string>
#include <ctype.h>

// SDL includes
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>


// Constantes
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600
#define SCREEN_BPP 32

// Surfaces
// Superficie principal
SDL_Surface *MainSurface = NULL;
// Superficie dos quadrados - olhar funcao LoadFiles()
SDL_Surface *SPRITE_BORDER = NULL;
SDL_Surface *SPRITE_BLUE   = NULL;
SDL_Surface *SPRITE_CYAN   = NULL;
SDL_Surface *SPRITE_GRAY   = NULL;
SDL_Surface *SPRITE_GREEN  = NULL;
SDL_Surface *SPRITE_RED    = NULL;
SDL_Surface *SPRITE_YELLOW = NULL;
SDL_Surface *SPRITE_ORANGE = NULL;

// Misc
SDL_Surface *LOGO = NULL;
SDL_Surface *TextPiece = NULL;
SDL_Surface *StartImage = NULL;
SDL_Surface *GameOver = NULL;

// Levels Surfaces
SDL_Surface *L1 = NULL;
SDL_Surface *L2 = NULL;
SDL_Surface *L3 = NULL;
SDL_Surface *L4 = NULL;
SDL_Surface *L5 = NULL;

using namespace std;

// Vars globla
bool Running = true;
SDL_Event event;
int CRoom = 1;		// irá controlar os estágios de execução.. 0 = game no start, 1 = game no select, 2 = game em play
int CLevel = 1;		// controle de linhas preenchidas
SDL_Window* _cWindow_ = NULL;
SDL_Surface* gScreenSurface = NULL;

// Timing
int VCount = 0;
int VCOUNT_MAX = 500;

// Color Constants
#define TCOLOR_BLUE 0
#define TCOLOR_CYAN 1
#define TCOLOR_GREEN 2
#define TCOLOR_ORANGE 3
#define TCOLOR_RED 4
#define TCOLOR_YELLOW 5
#define TCOLOR_GRAY 6

// Constantes para matrizes
#define MATRIX_DRAW_POS_X 100
#define MATRIX_DRAW_POS_Y 0
#define MATRIX_PIECES_X 10
#define MATRIX_PIECES_Y 20


// TPiece Class
class TPiece
{
public:
	int Color = 0; // Representa a cor blue (azul)
	bool Used = false;

};

// Cria uma Matriz para controlar as peças d jogo
// A matriz bidimensional PieceMatrix armazena cada
// bloco do tetris sendo 10x20
// A TmpMatrix é uma matriz 4x4 qe armazena temporariamente
// a peça que esta descendo para depois ser aplicada na PieceMatrix
TPiece PieceMatrix[MATRIX_PIECES_X][MATRIX_PIECES_Y];
TPiece TmpMatrix[4][4];
int XPosTmp = 4;
int YPosTmp = 0;
int WidthTmp = 0;
int HeightTmp = 0;

// Buttons States
bool DownButton = false;

// Para o gerador de peças
int CRAND = 0;
int DIF = 4;
int NextPiece = 4;

// Para a funcao de verificacao de linhas preenchidas
int Points = 0;

// Musica
Mix_Music *TetrisMusic = NULL;

// Prototipos de funcoes
SDL_Surface *LoadSurfaceFromFile(std::string filename);
void	    DrawSurface(int x, int y, SDL_Surface* source, SDL_Surface* destination);
void		DrawTetrisMatrix();
bool		LoadFiles();
void		UnloadFiles();
bool		Update();
void		ClearTmpMatrix();
void		GenerateTmpMatrix(int FigIndex);
void		FillTmpColor(int color);
bool		TestBlockColision();
void		PutInMatrix();
int         GetNewPieceIndex();
void        VerifyLines();
void		RotateTmpMatrix();
void		CalculateTmpSize();
void		SetMatrixInCorner();
void		ResetPieceMatrix();
void		DrawTmpPiece(int x, int y);
void		VerifyLines();


int main(int argc, char *argv[])
{
	// std out file write
	cout << "Carregando Tetris!" << endl;

	// Iniciando o mixer e a musica do jogo
	if (Mix_OpenAudio(48000, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
	{
		// erro de abertura do arquivo
		cout << "Erro na inicializacao do SDL Mix. Erro:" << Mix_GetError() << endl;
		return true;
	}
	
	// iniciando SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
	{
		// erro na carga do SDL
		cout << "Erro de inicialização do framework SDL!" << SDL_GetError() << endl;
		return 1;
	}
	else
	{
		// Criando a janela para as superficies
		_cWindow_ = SDL_CreateWindow("Tetris SDL2 by Claudecir",
											SDL_WINDOWPOS_CENTERED, 
											SDL_WINDOWPOS_CENTERED, 
											SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (_cWindow_ == NULL) {
			cout << "Erro de inicializacao da tela! SDL Erro: " << SDL_GetError();
			return -1;
		}
		else
		{
			// Criando uma superficie de jogo
			
			gScreenSurface = SDL_GetWindowSurface(_cWindow_);
		}
		//SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

	}
	// Leitura de arquivos 
	int imgFlags = IMG_INIT_PNG;
	if ( !( IMG_Init( imgFlags ) & imgFlags ) )
	{
		cout << "SDL_Image nao inicializado! Erro: " << SDL_GetError() << endl;
	}
	else {
		if ( LoadFiles() == true )
		{
			cout << "Erro na leitura de arquivos de imagens!" << endl;
			return 1;
		}
	}

	// Geracao de uma matriz temporaria
	GenerateTmpMatrix(0);

	// Play na musica do jogo
	if (Mix_PlayingMusic() == 0)
	{
		if (Mix_PlayMusic(TetrisMusic, -1) == -1)
		{
			cout << "Erro na execucao do arquivo de musica do jogo!" << Mix_GetError() << endl;
			return true;
		}
	}

	// inicio do loop do game
	while ( Running )
	{
		if (Update() == 1)
		{
			// Erro durante update
			Running = false;
		}
		while (SDL_PollEvent( &event ))
		{
			// eventos do teclado para a movimentação dos blocos
			// PAREI AQUI... .6:43
			if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_LEFT:
					if (XPosTmp > 0) XPosTmp -= 1;
					break;
				case SDLK_RIGHT:
					if (XPosTmp < MATRIX_PIECES_X - (WidthTmp + 1)) XPosTmp += 1;
					break;
				case SDLK_DOWN:
					DownButton = true;
					break;
				case SDLK_UP:
					// Rotate
					RotateTmpMatrix();
					break;
				case SDLK_RETURN:
					if (CRoom == 1)
						CRoom = 0;
					else if (CRoom == 2)
					{
						ResetPieceMatrix();
						YPosTmp = 0;
						XPosTmp = 5;
						GenerateTmpMatrix(0);
						CRoom = 1;
					}
					break;
				}
			}
			if (event.type == SDL_KEYUP)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_DOWN:
					DownButton = false;
					break;
				}
			}

			// Event Close
			if ( event.type == SDL_QUIT )
			{
				// Desativa o loop e fecha o programa
				Running = false;
			}
		}
		Update();
		SDL_UpdateWindowSurface(_cWindow_);
	}
	
	// Descarrega os arquivos
	UnloadFiles();

	// Destroy as janelas criadas
	SDL_DestroyWindow(_cWindow_);

	// encerra os subsistemas do framework
	SDL_Quit();
	IMG_Quit();
	Mix_Quit();

	return 0;
}

////// ATUALIZACOES A CADA CICLO DO JOGO
bool Update()
{
	// limpando a tela
	SDL_FillRect(gScreenSurface, NULL, 0x000000 );

	// Draw just the Selected Room
	if (CRoom == 2)
	{
		DrawSurface(100, 100, GameOver, gScreenSurface);

	}
	if (CRoom == 1)
	{
		DrawSurface(120, 100, StartImage, gScreenSurface);

	}
	// Desenha apenas o quarto selecionado
	if ( CRoom == 0 )
	{
		// Carregar imagem inicial e logo
		DrawSurface(350, 0, LOGO, gScreenSurface);
		DrawSurface(350, 130, TextPiece, gScreenSurface);

		// rotina para tratativa de pressionamento do botão DOWN do teclado
		// faz o bloco cair direto até o fim
		if (DownButton == true)
		{
			VCount += 20;
		}
		// Esta parte da funcao Update calcula o tempo para cada unidade de bloco
		// que desce no tetris além de chamar funcoes de deteccao de colisao
		// verificacao de linhas entre outas checagens
		if (VCount >= VCOUNT_MAX)
		{
			if (TestBlockColision() == true) 
			{
				// coloca o bloco do tmp na matrix do jogo
				PutInMatrix();
				// Reseta a posicao
				XPosTmp = 5;
				YPosTmp = 0;

				//
				int pInd = GetNewPieceIndex();
				GenerateTmpMatrix(pInd);
				//
				CalculateTmpSize();
				//
				VerifyLines();
			}
			else {
				YPosTmp += 1;
			}
			

			VCount = 0;
		}
		else {
			VCount++;
		}

		DrawTetrisMatrix();
	}


	return false;
}

//---- Funcoes de calculos das pecas do game


void ResetPieceMatrix()
{
	for (int x = 0; x < MATRIX_PIECES_X; x++)
	{
		for (int y = 0; y < MATRIX_PIECES_Y; y++)
		{
			PieceMatrix[x][y].Used = false;
		}
	}
}

// Função para gerar um indice de peças a ser gerada,
// esta com problema pois nao gera o I e o Z.... ARRUMAR DEPOIS E COLOCAR PARA GERAR PELO TIMER.H
int GetNewPieceIndex()
{
	int newp = CRAND + DIF;
	if (newp > 6)
	{
		newp = newp - 6;
	}
	CRAND = newp;
	//
	newp = CRAND + DIF;
	if (newp > 6)
	{
		newp = newp - 6;
	}
	NextPiece = newp;
	//
	return CRAND;
}

// A funcao DrawTetrisMatrix é a funcao responsavel por calcular as posicoes
// dos blocos e Mostrar cada bloco das Matrizes PieceMatrix 10x20 e TmpMatrix 4x4
// Tambem mostra as bordas da area dos blocos
void DrawTetrisMatrix()
{
	////// ESTAGIO 1
	// variavei da funcao
	int px_start = MATRIX_DRAW_POS_X + 20;   // + o tamanho do bloco
	int py_start = MATRIX_DRAW_POS_Y + 100;  // + espacos

	int rightBorderPosX = px_start + (MATRIX_PIECES_X * 20);
	int leftBorderPosX = MATRIX_DRAW_POS_X;
	//
	int DownBorderMaxX = px_start + (MATRIX_PIECES_X * 20);

	// Desenha bordas
	// Borda esquerda
	int YLimit = (MATRIX_PIECES_Y * 20) + 20 + py_start;
	for (int yp = 0; yp < YLimit; yp += 20)	// Adiciona +1 peça na borda // +1 = Limite
	{
		DrawSurface(MATRIX_DRAW_POS_X, yp, SPRITE_BORDER, gScreenSurface);     

	}
	// Borda direita
	for ( int yp = 0; yp < YLimit; yp += 20 )	// Adiciona +1 peça na borda // +1 = Limite
	{
		DrawSurface(rightBorderPosX, yp, SPRITE_BORDER, gScreenSurface);

	}
	// desenha a borda da base
	//for ( int xp = MATRIX_DRAW_POS_X; xp < (DownBorderMaxX+20); xp += 20 )
	for ( int xp = MATRIX_DRAW_POS_X; xp < DownBorderMaxX; xp += 20 )
	{
		DrawSurface( xp, YLimit - 20, SPRITE_BORDER, gScreenSurface);
	}

	////// ESTAGIO 2
	// Desenha as pecas (Draw pieces)
	for (int xp = 0; xp < MATRIX_PIECES_X; xp++)
	{
		for (int yp = 0; yp < MATRIX_PIECES_Y; yp++)
		{
			if (PieceMatrix[xp][yp].Used == true)		// colocar false para testar o codigo
			{
				int PosXG = px_start + (xp * 20);
				int PosYG = py_start + (yp * 20);
				if (PieceMatrix[xp][yp].Color == TCOLOR_BLUE)
				{
					DrawSurface(PosXG, PosYG, SPRITE_BLUE, gScreenSurface);
				}
				if (PieceMatrix[xp][yp].Color == TCOLOR_CYAN)
				{
					DrawSurface(PosXG, PosYG, SPRITE_CYAN, gScreenSurface);
				}
				if (PieceMatrix[xp][yp].Color == TCOLOR_GRAY)
				{
					DrawSurface(PosXG, PosYG, SPRITE_GRAY, gScreenSurface);
				}
				if (PieceMatrix[xp][yp].Color == TCOLOR_GREEN)
				{
					DrawSurface(PosXG, PosYG, SPRITE_GREEN, gScreenSurface);
				}
				if (PieceMatrix[xp][yp].Color == TCOLOR_ORANGE)
				{
					DrawSurface(PosXG, PosYG, SPRITE_ORANGE, gScreenSurface);
				}
				if (PieceMatrix[xp][yp].Color == TCOLOR_RED)
				{
					DrawSurface(PosXG, PosYG, SPRITE_RED, gScreenSurface);
				}
				if (PieceMatrix[xp][yp].Color == TCOLOR_YELLOW)
				{
					DrawSurface(PosXG, PosYG, SPRITE_YELLOW, gScreenSurface);
				}
				//
			}
		}
	}

	// Desenha a peca temporaria
	for (int xp = 0; xp < 4; xp++)
	{
		for (int yp = 0; yp < 4; yp++)
		{
			if (TmpMatrix[xp][yp].Used == true) // False for a test
			{
				//
				int PosXG = px_start + ((xp + XPosTmp) * 20);
				int PosYG = py_start + ((yp + YPosTmp) * 20);
				//
				if (TmpMatrix[xp][yp].Color == TCOLOR_BLUE)
				{
					DrawSurface(PosXG, PosYG, SPRITE_BLUE, gScreenSurface);
				}
				if (TmpMatrix[xp][yp].Color == TCOLOR_CYAN)
				{
					DrawSurface(PosXG, PosYG, SPRITE_CYAN, gScreenSurface);
				}
				if (TmpMatrix[xp][yp].Color == TCOLOR_GRAY)
				{
					DrawSurface(PosXG, PosYG, SPRITE_GRAY, gScreenSurface);
				}
				if (TmpMatrix[xp][yp].Color == TCOLOR_GREEN)
				{
					DrawSurface(PosXG, PosYG, SPRITE_GREEN, gScreenSurface);
				}
				if (TmpMatrix[xp][yp].Color == TCOLOR_ORANGE)
				{
					DrawSurface(PosXG, PosYG, SPRITE_ORANGE, gScreenSurface);
				}
				if (TmpMatrix[xp][yp].Color == TCOLOR_RED)
				{
					DrawSurface(PosXG, PosYG, SPRITE_RED, gScreenSurface);
				}
				if (TmpMatrix[xp][yp].Color == TCOLOR_YELLOW)
				{
					DrawSurface(PosXG, PosYG, SPRITE_YELLOW, gScreenSurface);
				}
				//
			}
		}
	}

	//
	//
	DrawTmpPiece(450, 180);

}




void DrawTmpPiece(int x, int y)
{
	// Draw Piece Tmp
	for (int xp = 0; xp < 4; xp++)
	{
		for (int yp = 0; yp < 4; yp++)
		{
			if (TmpMatrix[xp][yp].Used == true) // False for a test
			{
				//
				int PosXG = x + (xp * 20);
				int PosYG = y + (yp * 20);
				//
				if (TmpMatrix[xp][yp].Color == TCOLOR_BLUE)
				{
					DrawSurface(PosXG, PosYG, SPRITE_BLUE, gScreenSurface);
				}
				if (TmpMatrix[xp][yp].Color == TCOLOR_CYAN)
				{
					DrawSurface(PosXG, PosYG, SPRITE_CYAN, gScreenSurface);
				}
				if (TmpMatrix[xp][yp].Color == TCOLOR_GRAY)
				{
					DrawSurface(PosXG, PosYG, SPRITE_GRAY, gScreenSurface);
				}
				if (TmpMatrix[xp][yp].Color == TCOLOR_GREEN)
				{
					DrawSurface(PosXG, PosYG, SPRITE_GREEN, gScreenSurface);
				}
				if (TmpMatrix[xp][yp].Color == TCOLOR_ORANGE)
				{
					DrawSurface(PosXG, PosYG, SPRITE_ORANGE, gScreenSurface);
				}
				if (TmpMatrix[xp][yp].Color == TCOLOR_RED)
				{
					DrawSurface(PosXG, PosYG, SPRITE_RED, gScreenSurface);
				}
				if (TmpMatrix[xp][yp].Color == TCOLOR_YELLOW)
				{
					DrawSurface(PosXG, PosYG, SPRITE_YELLOW, gScreenSurface);
				}
				//
			}
		}
	}

}

// A funcao TestBlockColision é a responsável em detectar quando as peças
// da matriz temporaria se colidem com as peças da matriz 10x20 ou com a 
// borda, ao colidir, a matriz temporaria é aplicada na matriz PieceMatrix 10x20
// e em seguida é resetada para posição inicial com uma nova forma
bool TestBlockColision()
{
	if (YPosTmp == 20 - (HeightTmp + 1)) // -msize
	{
		return true;
	}
	for (int xpt = 0; xpt < 4; xpt++)
	{
		for (int ypt = 0; ypt < 4; ypt++)
		{
			if (TmpMatrix[xpt][ypt].Used == true)
			{
				//
				//
				for (int xp = 0; xp < MATRIX_PIECES_X; xp++)
				{
					for (int yp = 0; yp < MATRIX_PIECES_Y; yp++)
					{
						if (PieceMatrix[xp][yp].Used == true)
						{
							//
							if (xpt + XPosTmp == xp && ypt + (YPosTmp + 1) == yp)
							{
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}


// limpa a matriz de blocos temporaria colocando todos os status Used em FALSE
void ClearTmpMatrix()
{
	for (int xp = 0; xp < 4; xp++)
	{
		for (int yp = 0; yp < 4; yp++)
		{
			TmpMatrix[xp][yp].Used = false;
		}
	}
}

void FillTmpColor(int color)
{
	for (int xp = 0; xp < 4; xp++)
	{
		for (int yp = 0; yp < 4; yp++)
		{
			TmpMatrix[xp][yp].Color = color;
		}
	}
}


// Rotaciona o bloco, ativado pelo pressionar da tecla UP no momento do jogo.
// ela faz uma copia para uma outra matriz de 4x4 para depois colocar nas novas posicoes
// esta funcao precisa usar a funcao para colocar no canto para nao causar problemas com o
// sistema de colisão.
void RotateTmpMatrix()
{
	TPiece MatrixBackup[4][4];
	for (int xp = 0; xp < 4; xp++)
	{
		for (int yp = 0; yp < 4; yp++)
		{
			MatrixBackup[xp][yp] = TmpMatrix[xp][yp];
		}
	}
	//
	TmpMatrix[0][3] = MatrixBackup[3][3];
	TmpMatrix[0][2] = MatrixBackup[2][3];
	TmpMatrix[0][1] = MatrixBackup[1][3];
	TmpMatrix[0][0] = MatrixBackup[0][3];
	//
	TmpMatrix[3][3] = MatrixBackup[3][0];
	TmpMatrix[2][3] = MatrixBackup[3][1];
	TmpMatrix[1][3] = MatrixBackup[3][2];
	TmpMatrix[0][3] = MatrixBackup[3][3];
	//
	TmpMatrix[3][3] = MatrixBackup[3][0];
	TmpMatrix[3][2] = MatrixBackup[2][0];
	TmpMatrix[3][1] = MatrixBackup[1][0];
	TmpMatrix[3][0] = MatrixBackup[0][0];
	//
	TmpMatrix[0][0] = MatrixBackup[0][3];
	TmpMatrix[1][0] = MatrixBackup[0][2];
	TmpMatrix[2][0] = MatrixBackup[0][1];
	TmpMatrix[3][0] = MatrixBackup[0][0];
	//
	TmpMatrix[1][1] = MatrixBackup[1][2];
	TmpMatrix[1][2] = MatrixBackup[2][2];
	TmpMatrix[2][2] = MatrixBackup[2][1];
	TmpMatrix[2][1] = MatrixBackup[1][1];
	//
	SetMatrixInCorner();
	CalculateTmpSize();
}

// A funcao GenerateTmpMatrix é uma das mais importantes, é ela quem gera os blocos de cada figura
// do tetris na matriz temporária 4x4 TmpMatrix, cada figura é identificada por um indice,
// antes de comecar a adicionar os blocos a matriz temporaria, é necessário limpa-la
// para remover os blocos anteriores.
void GenerateTmpMatrix(int FigIndex)
{
	// formatos do corpo de blocos, cada numero representa um formato
	// 0 = I, 1 = J, 2 = l, 3 = O, 4 = Z, 5 = T, 6 = S
	ClearTmpMatrix();
	if (FigIndex == 0)
	{
		// I
		FillTmpColor(TCOLOR_BLUE);
		TmpMatrix[0][0].Used = true;
		TmpMatrix[0][1].Used = true;
		TmpMatrix[0][2].Used = true;
		TmpMatrix[0][3].Used = true;
	}
	if (FigIndex == 1)
	{
		// J
		FillTmpColor(TCOLOR_CYAN);
		TmpMatrix[1][0].Used = true;
		TmpMatrix[1][1].Used = true;
		TmpMatrix[1][2].Used = true;
		TmpMatrix[0][2].Used = true;
	}
	if (FigIndex == 2)
	{
		// L
		FillTmpColor(TCOLOR_GRAY);
		TmpMatrix[0][0].Used = true;
		TmpMatrix[0][1].Used = true;
		TmpMatrix[0][2].Used = true;
		TmpMatrix[1][2].Used = true;
	}
	if (FigIndex == 3)
	{
		// O
		FillTmpColor(TCOLOR_GREEN);
		TmpMatrix[0][0].Used = true;
		TmpMatrix[0][1].Used = true;
		TmpMatrix[1][1].Used = true;
		TmpMatrix[1][0].Used = true;
	}
	if (FigIndex == 4)
	{
		// Z
		FillTmpColor(TCOLOR_ORANGE);
		TmpMatrix[0][0].Used = true;
		TmpMatrix[1][0].Used = true;
		TmpMatrix[1][1].Used = true;
		TmpMatrix[2][1].Used = true;
	}
	if (FigIndex == 5)
	{
		// T
		FillTmpColor(TCOLOR_RED);
		TmpMatrix[1][0].Used = true;
		TmpMatrix[0][1].Used = true;
		TmpMatrix[1][1].Used = true;
		TmpMatrix[2][1].Used = true;
	}
	if (FigIndex == 6)
	{
		// S
		FillTmpColor(TCOLOR_YELLOW);
		TmpMatrix[0][1].Used = true;
		TmpMatrix[1][1].Used = true;
		TmpMatrix[1][0].Used = true;
		TmpMatrix[2][0].Used = true;
	}
	//
	CalculateTmpSize();
}


//
// Rotina que verifica se a ultima linha está totalmente preenchida pelos blocos 
// se estiver realiza a remocao da linha e baixa os blocos de cima para a linha de baixo
void VerifyLines()
{

	for (int yP = 20; yP > -1; yP--)
	{
		bool hU = false;
		for (int xP = 0; xP < MATRIX_PIECES_X; xP++)
		{
			if (PieceMatrix[xP][yP].Used == false)
			{
				hU = true;
			}
		}
		if (hU == false)
		{
			// Complete Line
			for (int xp = 0; xp < MATRIX_PIECES_X; xp++)
			{
				PieceMatrix[xp][yP].Used = false;
			}
			// Get Down Matrix
			for (int yPv = yP; yPv > -1; yPv--)
			{
				for (int xp = 0; xp < MATRIX_PIECES_X; xp++)
				{
					if (yPv - 1 > -1)
					{
						PieceMatrix[xp][yPv] = PieceMatrix[xp][yPv - 1];
					}
				}
			}
			//
			Points += 1;
			if (Points < 10)
			{
				CLevel = 1;
				VCOUNT_MAX = 500;
			}
			if (Points == 10)
			{
				CLevel = 2;
				VCOUNT_MAX = 400;
			}
			if (Points == 20)
			{
				CLevel = 3;
				VCOUNT_MAX = 300;
			}
			if (Points == 30)
			{
				CLevel = 4;
				VCOUNT_MAX = 200;
			}
			if (Points == 50)
			{
				CLevel = 5;
				VCOUNT_MAX = 100;
			}
		}
	}

}

// A funcao SetMatrixInCorner coloca todos os blocos da matrix temporaria 4x4 no canto
// superior esquerdo da matriz, local onde começa os valores 0x0.
// Também é necessário para o sistema de colisão.
void SetMatrixInCorner()
{
	TPiece MatrixBackup[5][5];
	// Left Side
	while (true)
	{
		for (int xp = 0; xp < 4; xp++)
		{
			for (int yp = 0; yp < 4; yp++)
			{
				MatrixBackup[xp][yp] = TmpMatrix[xp][yp];
			}
		}
		//
		bool h = false;
		for (int yTmp = 0; yTmp < 4; yTmp++)
		{
			if (TmpMatrix[0][yTmp].Used == true)
			{
				h = true;
			}
		}
		if (h == true)
		{
			break;
		}
		else
		{
			for (int xp = 0; xp < 4; xp++)
			{
				for (int yp = 0; yp < 4; yp++)
				{
					TmpMatrix[xp][yp] = MatrixBackup[xp + 1][yp];
				}
			}
		}
	}
	// Up Side
	while (true)
	{
		for (int xp = 0; xp < 4; xp++)
		{
			for (int yp = 0; yp < 4; yp++)
			{
				MatrixBackup[xp][yp] = TmpMatrix[xp][yp];
			}
		}
		//
		bool h = false;
		for (int xTmp = 0; xTmp < 4; xTmp++)
		{
			if (TmpMatrix[xTmp][0].Used == true)
			{
				h = true;
			}
		}
		if (h == true)
		{
			break;
		}
		else
		{
			for (int xp = 0; xp < 4; xp++)
			{
				for (int yp = 0; yp < 4; yp++)
				{
					TmpMatrix[xp][yp] = MatrixBackup[xp][yp + 1];
				}
			}
		}
	}

}


// transfere dados da matrix temporaria para a matrix do jogo, é chamada da funcao UpDate()
void PutInMatrix()
{
	for (int xpt = 0; xpt < 4; xpt++)
	{
		for (int ypt = 0; ypt < 4; ypt++)
		{
			if (TmpMatrix[xpt][ypt].Used == true)
			{
				if (xpt + XPosTmp > -1 && xpt + XPosTmp < MATRIX_PIECES_X + 1 &&
					ypt + YPosTmp > -1 && ypt + YPosTmp < MATRIX_PIECES_Y + 1)
				{
					PieceMatrix[xpt + XPosTmp][ypt + YPosTmp] = TmpMatrix[xpt][ypt];
				}
			}
		}
	}


}

// A função CalculateTmpSize calcula quantos blocos são ocupados na matriz
// temporaria 4x4 nos eixos x e y. Isso é necessário para saber o limite de
// movimentação da peça sem que elasaia da area do jogo.
void CalculateTmpSize()
{
	int width = 0;
	int height = 0;
	//
	for (int xp = 0; xp < 4; xp++)
	{
		for (int yp = 0; yp < 4; yp++)
		{
			if (TmpMatrix[xp][yp].Used == true)
			{
				if (xp > width)
				{
					width = xp;
				}
				if (yp > height)
				{
					height = yp;
				}
			}
		}
	}
	// variaveis definidas no topo do programa como globais
	WidthTmp = width;
	HeightTmp = height;
}


//----

// Funcao para ler os arquivos de imagens e sons do jogo
bool LoadFiles()
{
	// Inicio da leitura dos sprites
	SPRITE_BORDER = LoadSurfaceFromFile("E:/PastasCDS/GameDev/TetrisSDL/Debug/borda.png");
	SPRITE_BLUE   = LoadSurfaceFromFile("E:/PastasCDS/GameDev/TetrisSDL/Debug/p_blue.png");
	SPRITE_CYAN   = LoadSurfaceFromFile("E:/PastasCDS/GameDev/TetrisSDL/Debug/p_cyan.png");
	SPRITE_GRAY   = LoadSurfaceFromFile("E:/PastasCDS/GameDev/TetrisSDL/Debug/p_gray.png");
	SPRITE_GREEN  = LoadSurfaceFromFile("E:/PastasCDS/GameDev/TetrisSDL/Debug/p_green.png");
	SPRITE_ORANGE = LoadSurfaceFromFile("E:/PastasCDS/GameDev/TetrisSDL/Debug/p_orange.png");
	SPRITE_RED    = LoadSurfaceFromFile("E:/PastasCDS/GameDev/TetrisSDL/Debug/p_red.png");
	SPRITE_YELLOW = LoadSurfaceFromFile("E:/PastasCDS/GameDev/TetrisSDL/Debug/p_yellow.png");

	// LOGO, Textos, Imagem inicial (splash) e imagem de GameOver
	LOGO = LoadSurfaceFromFile("E:/PastasCDS/GameDev/TetrisSDL/Debug/logo.png");
	TextPiece = LoadSurfaceFromFile("E:/PastasCDS/GameDev/TetrisSDL/Debug/cp.png");
	StartImage = LoadSurfaceFromFile("E:/PastasCDS/GameDev/TetrisSDL/Debug/tetris_logo.png");
	GameOver = LoadSurfaceFromFile("E:/PastasCDS/GameDev/TetrisSDL/Debug/gameover.png");

	// Load music
	TetrisMusic = Mix_LoadMUS("E:/PastasCDS/GameDev/TetrisSDL/Debug/Tetris.mp3");

	return false;
}


void UnloadFiles()
{
	// Descarrega sprites e surface da memoria
	SDL_FreeSurface(SPRITE_BORDER);
	SDL_FreeSurface(SPRITE_BLUE);
	SDL_FreeSurface(SPRITE_CYAN);
	SDL_FreeSurface(SPRITE_GRAY);
	SDL_FreeSurface(SPRITE_GREEN);
	SDL_FreeSurface(SPRITE_ORANGE);
	SDL_FreeSurface(SPRITE_RED);
	SDL_FreeSurface(SPRITE_YELLOW);

	// Descarrega logo
	SDL_FreeSurface(LOGO);
	SDL_FreeSurface(TextPiece);
	SDL_FreeSurface(StartImage);
	SDL_FreeSurface(GameOver);

	// Free music - libera memoria da musica carregada
	Mix_FreeMusic(TetrisMusic);

}

// Funcao que desenha a imagem na surface destino onde está o jogo
void DrawSurface(int x, int y, SDL_Surface* source, SDL_Surface* destination)
{
	// Cria uma superficie retangula - area do jogo
	SDL_Rect offset;
	offset.x = x;
	offset.y = y;

	// Blit 
	SDL_BlitSurface(source, NULL, destination, &offset);
	
}

// Funcao para realizar a leitura fisica dos arquivos do game
// e renderizar as mesmas em memória
SDL_Surface *LoadSurfaceFromFile( std::string filename )
{
	// Superficies
	SDL_Surface* OptimizedImage = NULL;

	// Leitura das imagens
	SDL_Surface* LoadedImage = IMG_Load( filename.c_str() );
	if ( LoadedImage == NULL )
	{
		cout << "Falha de leitura do arquivo: " << filename.c_str() << "  Erro: " << IMG_GetError() << endl;
	} else
	{
		// debug
		//cout << "lido o: " << filename.c_str() << endl;

		// Criando uma imagem otimizada e convertendo para o formato da tela
		//optimizedImage = SDL_DisplayFormat(LoadedImage); // - linha original
		OptimizedImage = SDL_ConvertSurface( LoadedImage, gScreenSurface->format, NULL );    // - linha lazyfoo
		if ( OptimizedImage == NULL )
		{
			cout << "Nao foi possivel otimizar a imagem:" << filename.c_str() << "  Erro: " << SDL_GetError();
		}
		// Aplicando a imagem a superficie
		SDL_FreeSurface( LoadedImage );
	}

	// retorna imagem otimizada
	return OptimizedImage;
}

