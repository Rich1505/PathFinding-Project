#include <chrono>
#include <thread>
#include<raylib.h>
#include<queue>
#include<vector>
#include<algorithm>


#define WINDOW_LENGTH 1366
#define WINDOW_HEIGHT 768


std::pair<int, int> InizioTile = { 0,0 };
std::pair<int, int> FineTile = { 41,59 };

struct Nodo
{
public:

	int gVal;
	float hVal;
	float fVal;
	int y;
	int x;
	int stato;
	Nodo* parent;

	Nodo(int y, int x) noexcept
	{
		hVal = sqrt(pow(x - FineTile.second, 2) + pow(y - FineTile.first, 2)) * 1.5f;
		gVal = INT_MAX - 1;
		this->x = x;
		this->y = y;
		stato = 0;
		fVal = INT_MAX - 1;
		parent = nullptr;
	}

	Nodo(int y, int x, int gVal, int hVal, int fVal, int stato, float bestRoad, Nodo* parent) noexcept
	{
		this->gVal = gVal;
		this->hVal = hVal;
		this->fVal = fVal;
		this->y = y;
		this->x = x;
		this->stato = stato;
		this->parent = parent;
	}

	Nodo(const Nodo& n) noexcept
	{
		parent = n.parent;
		this->gVal = n.gVal;
		this->hVal = n.hVal;
		this->fVal = n.fVal;
		this->y = n.y;
		this->x = n.x;
		this->stato = n.stato;
	}

	Nodo(const Nodo&& n) noexcept
	{
		parent = n.parent;
		this->gVal = n.gVal;
		this->hVal = n.hVal;
		this->fVal = n.fVal;
		this->y = n.y;
		this->x = n.x;
		this->stato = n.stato;
	}

	Nodo() noexcept
	{
		gVal = 0;
		hVal = 0;
		fVal = 0;
		y = 0;
		x = 0;
		stato = 0;
		parent = nullptr;
	}

	bool operator>(const Nodo& b) const
	{
		return fVal > b.fVal;
	}

	bool operator<(const Nodo& b) const
	{
		return fVal < b.fVal;
	}

	void operator=(const Nodo& n)
	{
		parent = n.parent;
		this->gVal = n.gVal;
		this->hVal = n.hVal;
		this->fVal = n.fVal;
		this->y = n.y;
		this->x = n.x;
		this->stato = n.stato;
	}
};

//variabili globali
Color BackGroundColor{ 240,218,104,94 };
Rectangle map{ 34,34,1020,714 };

int mapHeight = 42;
int mapWidth = 60;
Nodo** tiles;

bool startThread = false;



std::priority_queue<Nodo, std::vector<Nodo>, std::greater<Nodo>> qu;

enum tileState {
	INESPLORATO, ESPLORATO, INIZIO, FINE, STRADA, MURO, LISTATO
};

void findRoad();
void generateRandom();
std::thread esplorazione(findRoad);
int explorationSpeed = 20;

void deleteMap(Nodo** tiles)
{
	for (int i = 0; i < mapHeight; i++)
	{
		delete[] tiles[i];
	}
	delete[] tiles;
}

void setupMap()
{

	tiles = new Nodo * [mapHeight];
	for (int i = 0; i < mapHeight; i++)
	{
		tiles[i] = new Nodo[mapWidth];
		for (int j = 0; j < mapWidth; j++)
		{
			tiles[i][j] = Nodo{ i,j };
			tiles[i][j].gVal = INT_MAX - 1;
		}
	}

	tiles[InizioTile.first][InizioTile.second].stato = INIZIO;
	tiles[FineTile.first][FineTile.second].stato = FINE;

	qu = std::priority_queue<Nodo, std::vector<Nodo>, std::greater<Nodo>>();

	tiles[InizioTile.first][InizioTile.second].gVal = 0;
	qu.push(tiles[InizioTile.first][InizioTile.second]);

}

void recalculateHval()
{
	Nodo** tilesTemp = new Nodo * [mapHeight];
	for (int i = 0; i < mapHeight; i++)
	{
		tilesTemp[i] = new Nodo[mapWidth];
		for (int j = 0; j < mapWidth; j++)
		{
			tilesTemp[i][j] = Nodo{ i,j };
			if (tiles[i][j].stato == MURO)
			{
				tilesTemp[i][j].stato = MURO;
				tilesTemp[i][j].gVal = INT_MIN;
			}
			else
			{
				tilesTemp[i][j].gVal = INT_MAX - 1;
			}
		}
	}

	tilesTemp[InizioTile.first][InizioTile.second].stato = INIZIO;
	tilesTemp[FineTile.first][FineTile.second].stato = FINE;

	qu = std::priority_queue<Nodo, std::vector<Nodo>, std::greater<Nodo>>();
	tilesTemp[InizioTile.first][InizioTile.second].gVal = 0;

	deleteMap(tiles);

	tiles = tilesTemp;


}

void drawMap()
{
	//disegna contorno e sfondo mappa
	DrawRectangle(map.x - 1, map.y - 1, map.width + 1, map.height + 1, WHITE);

	DrawRectangleLines(map.x - 1, map.y - 1, map.width + 1, map.height + 1, BLACK);




	//disegna i quadrati interni alla mappa

	float divisionY = map.height / mapHeight;
	for (int i = 1; i < mapHeight; i++)
	{
		int endingY = divisionY * i + map.y;
		DrawLineEx(Vector2{ map.x, (float)endingY }, Vector2{ map.x + map.width, (float)endingY }, 1, BLACK);
	}

	float divisionX = map.width / mapWidth;
	for (int i = 1; i < mapWidth; i++)
	{
		int endingX = divisionX * i + map.x;
		DrawLineEx(Vector2{ (float)endingX, map.y }, Vector2{ (float)endingX, map.y + map.height }, 1, BLACK);
	}

	//colora rettangoli

	for (int i = 0; i < mapHeight; i++)
	{
		for (int j = 0; j < mapWidth; j++)
		{
			switch (tiles[i][j].stato)
			{
			case tileState::INESPLORATO:

				DrawRectangle(j * divisionX + map.x + 1, i * divisionY + map.y + 1, divisionX - 3, divisionY - 3, LIGHTGRAY);
				break;

			case tileState::ESPLORATO:
				DrawRectangle(j * divisionX + map.x + 1, i * divisionY + map.y + 1, divisionX - 3, divisionY - 3, GRAY);
				break;

			case tileState::INIZIO:
				DrawRectangle(j * divisionX + map.x + 1, i * divisionY + map.y + 1, divisionX - 3, divisionY - 3, GREEN);
				DrawRectangleLines(j * divisionX + map.x + 1, i * divisionY + map.y + 1, divisionX - 3, divisionY - 3, BLACK);
				break;

			case tileState::FINE:
				DrawRectangle(j * divisionX + map.x + 1, i * divisionY + map.y + 1, divisionX - 3, divisionY - 3, RED);
				DrawRectangleLines(j * divisionX + map.x + 1, i * divisionY + map.y + 1, divisionX - 3, divisionY - 3, BLACK);
				break;

			case tileState::STRADA:
				DrawRectangle(j * divisionX + map.x + 1, i * divisionY + map.y + 1, divisionX - 3, divisionY - 3, DARKPURPLE);
				DrawRectangleLines(j * divisionX + map.x + 1, i * divisionY + map.y + 1, divisionX - 3, divisionY - 3, BLACK);
				break;

			case tileState::MURO:
				DrawRectangle(j * divisionX + map.x + 1, i * divisionY + map.y + 1, divisionX - 3, divisionY - 3, BLACK);
				break;

			case tileState::LISTATO:
				DrawRectangle(j * divisionX + map.x + 1, i * divisionY + map.y + 1, divisionX - 3, divisionY - 3, SKYBLUE);
				DrawRectangleLines(j * divisionX + map.x + 1, i * divisionY + map.y + 1, divisionX - 3, divisionY - 3, BLACK);
				break;
			}
			//DrawText(TextFormat("%.2f", tiles[i][j].fVal), j * divisionX + map.x + 1, i * divisionY + map.y + 1, 10, BLACK);
		}
	}

}



//disegna il selettore di grandezza della mappa

Rectangle mapSizeSelector{ map.x + map.width + (WINDOW_LENGTH - map.x - map.width - 200) / 2,map.y + 30,200,50 };

int selectedSize = 1;

void drawSelector()
{

	int textX = (mapSizeSelector.width - MeasureText("Grandezza mappa", 19)) / 2 + mapSizeSelector.x;
	DrawText("Grandezza mappa", textX, map.y, 19, BLACK);
	DrawRectangle(mapSizeSelector.x, mapSizeSelector.y, mapSizeSelector.width, mapSizeSelector.height, WHITE);

	switch (selectedSize)
	{
	case 1:
		DrawRectangle(mapSizeSelector.x, mapSizeSelector.y, 50, 50, GRAY);
		break;

	case 2:
		DrawRectangle(mapSizeSelector.x + 50, mapSizeSelector.y, 50, 50, GRAY);
		break;

	case 3:
		DrawRectangle(mapSizeSelector.x + 100, mapSizeSelector.y, 50, 50, GRAY);
		break;

	case 4:
		DrawRectangle(mapSizeSelector.x + 150, mapSizeSelector.y, 50, 50, GRAY);
		break;
	}

	DrawRectangleLines(mapSizeSelector.x, mapSizeSelector.y, mapSizeSelector.width, mapSizeSelector.height, BLACK);

	for (int i = 1; i < 4; i++)
	{
		DrawLine(mapSizeSelector.x + 50 * i, mapSizeSelector.y, mapSizeSelector.x + 50 * i, mapSizeSelector.y + mapSizeSelector.height, BLACK);
	}

	textX = (50 - MeasureText("1", 40)) / 2;
	DrawText("1", mapSizeSelector.x + textX, mapSizeSelector.y + 8, 40, BLACK);
	DrawText("2", mapSizeSelector.x + 50 + textX, mapSizeSelector.y + 8, 40, BLACK);
	DrawText("3", mapSizeSelector.x + 100 + textX, mapSizeSelector.y + 8, 40, BLACK);
	DrawText("4", mapSizeSelector.x + 150 + textX, mapSizeSelector.y + 8, 40, BLACK);
}

Rectangle firstSelection{ mapSizeSelector.x, mapSizeSelector.y, 50, 50 };
Rectangle secondSelection{ mapSizeSelector.x + 50,mapSizeSelector.y,50,50 };
Rectangle thirdSelection{ mapSizeSelector.x + 100, mapSizeSelector.y, 50, 50 };
Rectangle fourthSelection{ mapSizeSelector.x + 150,mapSizeSelector.y,50,50 };

Rectangle Inizio{ map.x + map.width + 50,WINDOW_HEIGHT / 2,100,50 };
Rectangle Fine{ Inizio.x + Inizio.width + 20,Inizio.y,Inizio.width,Inizio.height };
Rectangle Muro{ Inizio.x + (Inizio.width + Fine.width + 20 - 100) / 2,Inizio.y + 75,Inizio.width,Inizio.height };

Rectangle Start{ Inizio.x + (Inizio.width + Fine.width + 20 - 200) / 2,Muro.y + Muro.height + 50, 200,50 };

Rectangle random{ Start.x,Inizio.y - Inizio.width,Start.width,Start.height };

Rectangle speedSelector{ Start.x,Start.y + Start.height + 80,Start.width,20 };
Rectangle speedSlider{ speedSelector.x + speedSelector.width - explorationSpeed - 50,speedSelector.y - 3,50,speedSelector.height + 6 };

int inputSelezionato = MURO;

bool roadFound = false;

// processa gli input dell'utente
bool processInput()
{
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
	{
		if (CheckCollisionPointRec(GetMousePosition(), map))
		{
			startThread = false;
			int x = (GetMousePosition().x - map.x) / (map.width / mapWidth);
			int y = (GetMousePosition().y - 1 - map.y) / (map.height / mapHeight);

			switch (inputSelezionato)
			{
			case MURO:
				if (tiles[y][x].stato != INIZIO && tiles[y][x].stato != FINE)
				{
					tiles[y][x].stato = MURO;
					tiles[y][x].gVal = INT_MIN;
				}
				break;

			case INIZIO:
				if (tiles[y][x].stato != FINE)
				{
					tiles[InizioTile.first][InizioTile.second].stato = INESPLORATO;
					tiles[y][x].stato = INIZIO;
					InizioTile.first = y;
					InizioTile.second = x;
					recalculateHval();
				}
				break;

			case FINE:
				if (tiles[y][x].stato != INIZIO)
				{
					tiles[FineTile.first][FineTile.second].stato = INESPLORATO;
					tiles[y][x].stato = FINE;
					FineTile.first = y;
					FineTile.second = x;
					recalculateHval();
				}
				break;
			}
			return true;
		}
		else if (CheckCollisionPointRec(GetMousePosition(), speedSlider))
		{
			int mouseX = GetMouseX();

			speedSlider.x = mouseX - speedSlider.width / 2;

			if (speedSlider.x < speedSelector.x)
			{
				speedSlider.x = speedSelector.x;
			}
			else if (speedSlider.x > speedSelector.x + speedSelector.width - speedSlider.width)
			{
				speedSlider.x = speedSelector.x + speedSelector.width - speedSlider.width;
			}

			explorationSpeed = 200 - (speedSlider.x - speedSelector.x + speedSlider.width);
		}
		else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			if ((CheckCollisionPointRec(GetMousePosition(), mapSizeSelector)))
			{
				startThread = false;
				deleteMap(tiles);

				if (CheckCollisionPointRec(GetMousePosition(), firstSelection))
				{
					InizioTile.first = 0;
					InizioTile.second = 0;
					FineTile.first = 41;
					FineTile.second = 59;

					map.width = 1020;
					map.height = 714;
					selectedSize = 1;
					mapWidth = 60;
					mapHeight = 42;
					setupMap();
				}
				else if (CheckCollisionPointRec(GetMousePosition(), secondSelection))
				{
					InizioTile.first = 0;
					InizioTile.second = 0;
					FineTile.first = 27;
					FineTile.second = 39;

					map.width = 1000;
					map.height = 700;
					selectedSize = 2;
					mapWidth = 40;
					mapHeight = 28;
					setupMap();
				}
				else if (CheckCollisionPointRec(GetMousePosition(), thirdSelection))
				{
					InizioTile.first = 0;
					InizioTile.second = 0;
					FineTile.first = 13;
					FineTile.second = 19;

					map.width = 1000;
					map.height = 700;
					selectedSize = 3;
					mapWidth = 20;
					mapHeight = 14;
					setupMap();
				}
				else if (CheckCollisionPointRec(GetMousePosition(), fourthSelection))
				{
					InizioTile.first = 0;
					InizioTile.second = 0;
					FineTile.first = 8;
					FineTile.second = 12;

					map.width = 1000;
					map.height = 700;
					selectedSize = 4;
					mapWidth = 13;
					mapHeight = 9;
					setupMap();
				}
				return true;
			}
			else if (CheckCollisionPointRec(GetMousePosition(), Inizio))
			{
				inputSelezionato = INIZIO;
			}
			else if (CheckCollisionPointRec(GetMousePosition(), Fine))
			{
				inputSelezionato = FINE;
			}
			else if (CheckCollisionPointRec(GetMousePosition(), Muro))
			{
				inputSelezionato = MURO;
			}
			else if (CheckCollisionPointRec(GetMousePosition(), Start))
			{

				for (int i = 0; i < mapHeight; i++)
				{
					for (int j = 0; j < mapWidth; j++)
					{
						if (tiles[i][j].stato == ESPLORATO || tiles[i][j].stato == INESPLORATO || tiles[i][j].stato == STRADA || tiles[i][j].stato == LISTATO)
						{
							tiles[i][j].stato = INESPLORATO;
							tiles[i][j].gVal = INT_MAX - 1;
							tiles[i][j].fVal = tiles[i][j].gVal + tiles[i][j].hVal;
						}
						else if (tiles[i][j].stato == FINE)
						{
							tiles[i][j].gVal = INT_MAX - 1;
							tiles[i][j].fVal = tiles[i][j].gVal + tiles[i][j].hVal;
						}

					}
				}

				tiles[InizioTile.first][InizioTile.second].gVal = 0;
				tiles[InizioTile.first][InizioTile.second].fVal = tiles[InizioTile.first][InizioTile.second].gVal + tiles[InizioTile.first][InizioTile.second].hVal;

				qu = std::priority_queue<Nodo, std::vector<Nodo>, std::greater<Nodo>>();
				qu.push(tiles[InizioTile.first][InizioTile.second]);

				roadFound = false;
				startThread = true;
			}
			else if (CheckCollisionPointRec(GetMousePosition(), random))
			{
				startThread = false;
				generateRandom();
			}
		}
	}
	else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && CheckCollisionPointRec(GetMousePosition(),map))
	{

		int x = (GetMousePosition().x - map.x - 1) / (map.width / mapWidth);
		int y = (GetMousePosition().y - map.y - 1) / (map.height / mapHeight);

		if (tiles[y][x].stato == MURO)
		{
			tiles[y][x].stato = INESPLORATO;
			tiles[y][x].gVal = INT_MAX - 1;
			tiles[y][x].fVal = INT_MAX - 1;
		}
	}
}

void drawSpeedSelector()
{
	int textX = (speedSelector.width - MeasureText("Velocita'", 20)) / 2;

	DrawText("Velocita'", speedSelector.x + textX, speedSelector.y - 35, 20, BLACK);

	DrawRectangleRec(speedSelector, WHITE);
	DrawRectangleLinesEx(speedSelector, 1, BLACK);

	DrawRectangleRec(speedSlider, BLACK);
}

//disegna il selettore di modalità di disegno
void drawTypeSelector()
{
	int textX = (Inizio.width - MeasureText("Inizio", 20)) / 2;

	DrawRectangleRec(Inizio, (inputSelezionato == INIZIO) ? GRAY : GREEN);
	DrawRectangleLines(Inizio.x, Inizio.y, Inizio.width, Inizio.height, BLACK);
	DrawText("Inizio", Inizio.x + textX, Inizio.y + 15, 20, BLACK);

	textX = (Inizio.width - MeasureText("Fine", 20)) / 2;
	DrawRectangleRec(Fine, (inputSelezionato == FINE) ? GRAY : RED);
	DrawRectangleLines(Fine.x, Fine.y, Fine.width, Fine.height, BLACK);
	DrawText("Fine", Fine.x + textX, Fine.y + 15, 20, BLACK);

	textX = (Inizio.width - MeasureText("Muro", 20)) / 2;
	DrawRectangleRec(Muro, (inputSelezionato == MURO) ? GRAY : YELLOW);
	DrawRectangleLines(Muro.x, Muro.y, Muro.width, Muro.height, BLACK);
	DrawText("Muro", Muro.x + textX, Muro.y + 15, 20, BLACK);

}

//disegna bottone inizia esplorazione
void drawStart()
{
	DrawRectangleRec(Start, (startThread) ? GRAY : RAYWHITE);
	DrawRectangleLines(Start.x, Start.y, Start.width, Start.height, BLACK);

	int textX = (Start.width - MeasureText("Inizia esplorazione", 20)) / 2;

	DrawText("Inizia esplorazione", Start.x + textX, Start.y + 15, 20, BLACK);
}

//disegna bottone genera random
void drawRandomGeneration()
{

	DrawRectangleRec(random, WHITE);
	DrawRectangleLinesEx(random, 1, BLACK);

	int textX = MeasureText("Genera random", 20);

	DrawText("Genera random", random.x + (random.width - textX) / 2, random.y + 15, 20, BLACK);


}

//genera in modo casuale delle mura nella mappa
void generateRandom()
{
	int rand;
	for (int i = 0; i < mapHeight; i++)
	{
		for (int j = 0; j < mapWidth; j++)
		{
			if (tiles[i][j].stato != INIZIO && tiles[i][j].stato != FINE)
			{
				rand = GetRandomValue(0, 100);
				if (rand <= 25)
				{
					tiles[i][j].stato = MURO;
					tiles[i][j].gVal = INT_MIN + 1;
					tiles[i][j].fVal = tiles[i][j].gVal + tiles[i][j].hVal;
				}
				else
				{
					tiles[i][j].stato = INESPLORATO;
					tiles[i][j].gVal = INT_MAX - 1;
					tiles[i][j].fVal = tiles[i][j].gVal + tiles[i][j].hVal;
				}
			}
		}
	}
}

bool killThread = false;

//trova la strada dall'inizio alla fine, gestito da un thread a parte
void findRoad()
{

	while (true)
	{
		if (startThread)
		{
			tiles[InizioTile.first][InizioTile.second].gVal = 0;

			//seleziona la fase di ricerca: strada trovata o non
			if (roadFound)
			{
				Nodo nodoCurr = qu.top();
				if (nodoCurr.x == InizioTile.second && nodoCurr.y == InizioTile.first)
				{
					startThread = false;
					continue;
				}
				qu.pop();


				if (tiles[nodoCurr.y][nodoCurr.x].y != FineTile.first || tiles[nodoCurr.y][nodoCurr.x].x != FineTile.second)
				{
					tiles[nodoCurr.y][nodoCurr.x].stato = STRADA;
				}

				Nodo next = *(nodoCurr.parent);

				qu.push(next);

				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			else if (!qu.empty())
			{
				Nodo nodoCurr = qu.top();
				tiles[nodoCurr.y][nodoCurr.x].gVal = INT_MIN + 1;
				qu.pop();

				if (nodoCurr.x == FineTile.second && nodoCurr.y == FineTile.first)
				{
					roadFound = true;

					qu = std::priority_queue<Nodo, std::vector<Nodo>, std::greater<Nodo>>();
					qu.push(tiles[FineTile.first][FineTile.second]);
					continue;
				}

				if (tiles[nodoCurr.y][nodoCurr.x].stato != INIZIO && tiles[nodoCurr.y][nodoCurr.x].stato != FINE)
					tiles[nodoCurr.y][nodoCurr.x].stato = ESPLORATO;


				if (nodoCurr.y > 0)
				{
					Nodo* next = &tiles[nodoCurr.y - 1][nodoCurr.x];
					if (nodoCurr.gVal + 1 < next->gVal)
					{
						next->gVal = nodoCurr.gVal + 1;
						next->fVal = next->gVal + next->hVal;
						next->parent = &tiles[nodoCurr.y][nodoCurr.x];
						next->stato = LISTATO;
						qu.push(*next);
					}
				}

				if (nodoCurr.x < mapWidth - 1)
				{
					Nodo* next = &tiles[nodoCurr.y][nodoCurr.x + 1];
					if (nodoCurr.gVal + 1 < next->gVal)
					{
						next->gVal = nodoCurr.gVal + 1;
						next->fVal = next->gVal + next->hVal;
						next->parent = &tiles[nodoCurr.y][nodoCurr.x];
						next->stato = LISTATO;
						qu.push(*next);
					}
				}

				if (nodoCurr.y < mapHeight - 1)
				{
					Nodo* next = &tiles[nodoCurr.y + 1][nodoCurr.x];
					if (nodoCurr.gVal + 1 < next->gVal)
					{
						next->gVal = nodoCurr.gVal + 1;
						next->fVal = next->gVal + next->hVal;
						next->parent = &tiles[nodoCurr.y][nodoCurr.x];
						next->stato = LISTATO;
						qu.push(*next);
					}
				}

				if (nodoCurr.x > 0)
				{
					Nodo* next = &tiles[nodoCurr.y][nodoCurr.x - 1];
					if (nodoCurr.gVal + 1 < tiles[nodoCurr.y][nodoCurr.x - 1].gVal)
					{
						next->gVal = nodoCurr.gVal + 1;
						next->fVal = next->gVal + next->hVal;
						next->parent = &tiles[nodoCurr.y][nodoCurr.x];
						next->stato = LISTATO;
						qu.push(*next);
					}
				}

				tiles[FineTile.first][FineTile.second].stato = FINE;
				std::this_thread::sleep_for(std::chrono::milliseconds(explorationSpeed));
			}
			else
			{
				startThread = false;
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
		}


		if (killThread)
			return;
	}


}

int main()
{

	InitWindow(WINDOW_LENGTH, WINDOW_HEIGHT, "PathFinding Project");
	SetTargetFPS(60);

	setupMap();

	SetRandomSeed(time(NULL));

	while (!WindowShouldClose())
	{
		BeginDrawing();

		ClearBackground(BackGroundColor);

		drawMap();

		drawSelector();

		drawTypeSelector();

		drawStart();

		drawRandomGeneration();

		drawSpeedSelector();

		EndDrawing();

		processInput();

	}

	CloseWindow();
	killThread = true;
	esplorazione.join();

	return 0;
}