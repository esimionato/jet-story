#include "Room.h"
#include <algorithm>
#include <cassert>
#include <functional>
#include <sstream>
#include <vector>
#include <Windows.h>
#include "../Shared/Engine.h"
#include "Collectibles.h"
#include "Enemies.h"
#include "Maze.h"
#include "Player.h"
#include "Projectiles.h"
#include "Settings.h"
#include "Sounds.h"


Room::~Room()
{
	for_each(m_entities.begin(), m_entities.end(), DeleteFunctor());
}


void Room::Init(const unsigned char* pRoomInitSequence, const unsigned char* pEntitiesInitSequence, const Textures& textures)
{
	// This function uses binary data extracted from the original game to build the maze and place the entities

	int row	= 0;
	int column = 0;

	while (unsigned char byte = *pRoomInitSequence++)
	{
		int block = byte & 0x1F;
		if (block > 0)
		{
			--block;
		}
		else
		{
			block = Maze::BLOCKS - 1;
		}

		int nBlocks = (byte & 0xE0) >> 5;

		for (int i = 0; (i < nBlocks) && (row < ROWS); ++i)
		{
			m_block[row][column] = block;

			switch (block)
			{
			case 3:
				AddObstacle(row, column,	0, 0, 32, 16);
				break;

			case 4:
				AddObstacle(row, column,	8,  0, 32, 16);
				AddObstacle(row, column,	8, 16, 16, 32);
				break;

			case 5:
				AddObstacle(row, column,	 0,  0, 24, 16);
				AddObstacle(row, column,	16, 16, 24, 32);
				break;

			case 6:
				AddObstacle(row, column,	0, 24, 32, 32);
				break;

			case 13:
				AddObstacle(row, column,	0,  0, 32,  8);
				AddObstacle(row, column,	0, 24, 32, 32);
				break;

			case Maze::BLOCKS - 1:
				break;

			default:
				AddObstacle(row, column,	0, 0, 32, 32);
				break;
			}

			++column;
			if (column >= COLUMNS)
			{
				column = 0;
				++row;
			}
		}
	}

	assert(row == ROWS);
	assert(column == 0);

	Entity* pPlatform = nullptr;

	m_entities.reserve(eMaxEntitiesPerRoom);

	for (int i = 0; i < eInitialEntitiesPerRoom; ++i)
	{
		int y    = *pEntitiesInitSequence++;
		int x    = *pEntitiesInitSequence++;
		            pEntitiesInitSequence++;
		int type = *pEntitiesInitSequence++;

		if ((type == 2) || (type == 3))
		{
			y -= 16;	// This is a platform; lower it
		}

		if (type != 0)
		{
			Entity* pEntity = CreateEntity(type, Vector2(float(x), float(y)), textures);
			if (pEntity)
			{
				AddGameEntity(pEntity);

				if (pPlatform)
				{
					// Platform entity is followed by its user entity
					pEntity->SetPlatform(pPlatform);
					pPlatform = nullptr;
				}
				else
				{
					if ((type == 2) || (type == 3))
					{
						pPlatform = pEntity;
					}
				}
			}
			else
			{
				// The map file contains errors; comment out the assert
				//assert(!!!"Entity should be non-zero!");
			}
		}
	}
}


void Room::AddObstacle(int row, int column, int left, int top, int right, int bottom)
{
	RECT rect = {
		32 * column		+ left,
		32 * (row + 1)	+ top,
		32 * column		+ right,
		32 * (row + 1)	+ bottom
	};

	assert(0 <= rect.left);
	assert(rect.right <= Engine::eScreenWidthInPixels);
	assert(0 <= rect.top);
	assert(rect.bottom <= Engine::eScreenHeightInPixels);

	m_obstacles.push_back(rect);
}


bool Room::OverlapsObstacles(const Entity* pEntity, float x, float y) const
{
	LONG left = static_cast<LONG>(x);
	LONG top  = static_cast<LONG>(y);
	RECT rect = { left, top, left + pEntity->GetWidth(), top + pEntity->GetHeight() };

	RECT rectCollision;

	for (Obstacles::const_iterator it = m_obstacles.begin(); it != m_obstacles.end(); ++it)
	{
		const RECT& rectObstacle = *it;
		if (IntersectRect(&rectCollision, &rect, &rectObstacle))
		{
			return true;
		}
	}

	return false;
}


Entity* Room::CreateEntity(int type, const Vector2& vInitialPos, const Textures& textures)
{
	Entity* pEntity = nullptr;

	switch (type)
	{
	case 0:
	case 5:
	case 6:
	case 7:
	case 8:
		return 0;

	case 1:			pEntity = new DualCannon    (vInitialPos);		break;
	case 2:			pEntity = new Platform      (vInitialPos);		break;
	case 3:			pEntity = new Platform2     (vInitialPos);		break;
	case 4:			pEntity = new BigBall       (vInitialPos);		break;
	case 10:		pEntity = new Globe         (vInitialPos);		break;
	case 11:		pEntity = new Medusa        (vInitialPos);		break;
	case 13:		pEntity = new Revolver      (vInitialPos);		break;
	case 14:		pEntity = new BigZ          (vInitialPos);		break;
	case 15:		pEntity = new Cannon        (vInitialPos);		break;
	case 16:		pEntity = new FlyingCannon  (vInitialPos);		break;
	case 17:		pEntity = new Flier         (vInitialPos);		break;
	case 18:		pEntity = new Flier2        (vInitialPos);		break;
	case 19:		pEntity = new RevolvingRadar(vInitialPos);		break;
	case 20:		pEntity = new RocketLauncher(vInitialPos);		break;
	case 21:		pEntity = new LeftRocket    (vInitialPos);		break;
	case 22:		pEntity = new RightRocket   (vInitialPos);		break;
	case 23:		pEntity = new Brick         (vInitialPos);		break;
	case 24:		pEntity = new Factory       (vInitialPos);		break;
	case 25:		pEntity = new Bomber        (vInitialPos);		break;
	case 26:		pEntity = new Rocket        (vInitialPos);		break;
	case 27:		pEntity = new Radiator    (vInitialPos, true);	break;	// Left Radiator
	case 28:		pEntity = new Radiator    (vInitialPos, false);	break;	// Right Radiator
	case 29:		pEntity = new Radar         (vInitialPos);		break;
	case 30:		pEntity = new Base          (vInitialPos);		break;

	case 31:		pEntity = new Fuel       (vInitialPos);		break;
	case 32:		pEntity = new Ammo       (vInitialPos);		break;
	case 33:		pEntity = new Bombs      (vInitialPos);		break;
	case 34:		pEntity = new Missiles   (vInitialPos);		break;
	case 35:		pEntity = new Balls      (vInitialPos);		break;
	case 36:		pEntity = new Shield     (vInitialPos);		break;
	case 37:		pEntity = new RandomCollectible(vInitialPos);break;
	case 38:		pEntity = new Stars      (vInitialPos);		break;

	default:		pEntity = new Entity(vInitialPos);
	}

	int width = (type == 1) ? 32 : (type == 4) ? 0 : 16;
	pEntity->SetTexture(textures[type], width);

	return pEntity;
}


void Room::Reset()
{
	RemoveEntities(Entity::eLT_Session);
	for_each(m_entities.begin(), m_entities.end(), std::mem_fun(&Entity::Reset));
}


void Room::Render(const Sprites& blockSprites) const
{
	for (int row = 0; row < ROWS; ++row)
	{
		for (int column = 0; column < COLUMNS; ++column)
		{
			int block = m_block[row][column];

			if (block >= static_cast<int>(blockSprites.size()))
			{
				assert(!!!"Incorrect block number detected!");
				return;
			}

			Sprite* pBlock = blockSprites[block];
			pBlock->SetPos(32.f * column, 32.f * (row + 1));
			pBlock->Render();
		}
	}


	for (Entities::const_iterator it = m_entities.begin(); it != m_entities.end(); ++it)
	{
		Entity* pEntity = *it;
		
		if (pEntity->IsEnabled())
		{
			pEntity->Render();
		}
	}
}


void Room::Update()
{
	m_entities.insert(m_entities.end(), m_entitiesToAdd.begin(), m_entitiesToAdd.end());
	m_entitiesToAdd.clear();

	MoveEntitiesAndResolveCollisions();

	UpdateEntities();

	CollectGarbageEntities();
}


void Room::MoveEntitiesAndResolveCollisions()
{
	Player& player = Player::GetPlayer();

	// Detect collision with maze walls
	for (Entities::iterator it = m_entities.begin(); it != m_entities.end(); )
	{
		Entity* pEntity = *it;
		assert(pEntity);

		if (!pEntity->IsEnabled() || !pEntity->IsMovable())
		{
			++it;
			continue;
		}

		Vector2 pos = pEntity->GetPos();
		//TODO Fix assertion failed when player is dying in room (2, 0) and flying into Base
		assert(!OverlapsObstacles(pEntity, pos.x, pos.y));

		Vector2 vel = pEntity->GetVelocity();
		Vector2 acc = pEntity->GetSteering();
		Vector2 newVel = vel + acc;
		Vector2 newPos = pEntity->GetPos() + vel + 0.5f * acc;

		bool bHorizonalCollision = (int(newPos.x) < 0) || (int(newPos.x) + pEntity->GetWidth() > Engine::eScreenWidthInPixels)
			? (pEntity != &player)	// Only player may leave the room
			: OverlapsObstacles(pEntity, newPos.x, pos.y);

		bool bVerticalCollision = (int(newPos.y) < 4 * 8) || (int(newPos.y) + pEntity->GetHeight() > Engine::eScreenHeightInPixels)
			? (pEntity != &player)	// Only player may leave the room
			: bVerticalCollision = OverlapsObstacles(pEntity, pos.x, newPos.y);

		// Detect diagonal collision
		if (!bHorizonalCollision && !bVerticalCollision)
		{
			bHorizonalCollision = bVerticalCollision = OverlapsObstacles(pEntity, newPos.x, newPos.y);
		}

		if (bHorizonalCollision || bVerticalCollision)
		{
			if (pEntity->GetLifeTime() == Entity::eLT_Collision)
			{
				delete pEntity;
				it = m_entities.erase(it);
				continue;
			}
			else
			{
				pEntity->OnCollision(bHorizonalCollision, bVerticalCollision);
				newVel = pEntity->GetVelocity() + acc;
			}
		}

		if (!bHorizonalCollision)
		{
			pos.x = newPos.x;
		}
		if (!bVerticalCollision)
		{
			pos.y = newPos.y;
		}

		pEntity->SetPos(pos);
		pEntity->SetVelocity(newVel);

		assert(!OverlapsObstacles(pEntity, pos.x, pos.y));

		++it;
	}

	bool bPlayerVsEnemyCollision = false;

	// Detect collisions with each other
	for (Entities::iterator it1 = m_entities.begin(); it1 != m_entities.end(); ++it1)
	{
		Entity* pEntity1 = *it1;

		if (!pEntity1->IsEnabled())
			continue;

		if (pEntity1 == &player)
		{
			if (player.GetShield() >= 0)
			{
				for (Entities::iterator it2 = m_entities.begin(); it2 != m_entities.end(); ++it2)
				{
					Entity* pEntity2 = *it2;

					if (!pEntity2->IsEnabled())
						continue;

					if ((pEntity2 != &player) && pEntity2->CollidesWithPlayer() && pEntity2->Overlaps(&player))
					{
						pEntity2->OnCollision(player);

						if (pEntity2->IsEnemy())
						{
							bPlayerVsEnemyCollision = true;
						}
					}
				}
			}
		}
		else
		{
			if (pEntity1->IsEnemy())
			{
				Enemy* pEnemy = static_cast<Enemy*>(pEntity1);

				for (Entities::iterator it2 = m_entities.begin(); it2 != m_entities.end(); ++it2)
				{
					Entity* pEntity2 = *it2;

					if (!pEntity2->IsEnabled())
						continue;

					if ((pEntity1 != pEntity2) && pEntity2->CollidesWithEnemy() && pEntity2->Overlaps(pEntity1))
					{
						pEntity2->OnCollision(*pEnemy);
					}
				}
			}
		}
	}

	if (bPlayerVsEnemyCollision && (player.GetShield() >= 0) && (Maze::GetMaze().GetBasesLeft() > 0))
	{
		if (!Audio::IsPlaying(Sounds::LESS_SHIELD))
		{
			Audio::Play(Sounds::LESS_SHIELD);
		}
	}
	else
	{
		if (Audio::IsPlaying(Sounds::LESS_SHIELD))
		{
			Audio::Stop(Sounds::LESS_SHIELD);
		}
	}
}


void Room::UpdateEntities()
{
	for (Entities::iterator it = m_entities.begin(); it != m_entities.end(); )
	{
		Entity* pEntity = *it;

		if (!pEntity->IsGarbage() && pEntity->IsEnabled())
		{
			pEntity->Update();

			pEntity->Animate();
			if (pEntity->IsGarbage())
			{
				it = m_entities.erase(it);
				continue;;
			}
		}

		++it;
	}
}


void Room::CollectGarbageEntities()
{
	bool bCurrentRoomIsEmpty = true;

	for (Entities::iterator it = m_entities.begin(); it != m_entities.end(); )
	{
		Entity* pEntity = *it;

		if (pEntity->IsGarbage())
		{
			delete pEntity;
			it = m_entities.erase(it);
		}
		else
		{
			if (pEntity->IsEnabled())
			{
				bCurrentRoomIsEmpty = false;
			}

			++it;
		}
	}

	if (bCurrentRoomIsEmpty)
	{
		if (Settings::bSoundEffects && !Settings::bMusic)
		{
			if (!Audio::IsPlaying(Sounds::EMPTY_ROOM))
			{
				Audio::Play(Sounds::EMPTY_ROOM, -1);
			}
		}
	}
	else
	{
		if (Audio::IsPlaying(Sounds::EMPTY_ROOM))
		{
			Audio::Stop(Sounds::EMPTY_ROOM);
		}
	}
}


void Room::OnPlayerEnter()
{
	Player& player = Player::GetPlayer();
	AddEntity(&player);
}


void Room::OnPlayerExit()
{
	Player& player = Player::GetPlayer();

	for (Entities::iterator it = m_entities.begin(); it != m_entities.end(); )
	{
		Entity* pEntity = *it;
		if ((pEntity == &player) || (pEntity->GetLifeTime() == Entity::eLT_FollowPlayer))
		{
			it = m_entities.erase(it);
		}
		else
		{
			++it;
		}
	}

	RemoveEntities(Entity::eLT_Room);
}


void Room::AddEntity(Entity* pEntity)
{
	m_entitiesToAdd.push_back(pEntity);
}


void Room::AddGameEntity(Entity* pEntity)
{
	assert(pEntity && (m_entities.size() < eMaxEntitiesPerRoom));
	if (pEntity && (m_entities.size() < eMaxEntitiesPerRoom))
	{
		pEntity->SetLifeTime(Entity::eLT_Game);
		m_entities.push_back(pEntity);
	}
}


void Room::AddDebris(const Vector2& pos)
{
	for (int i = 0; i < 8; ++i)
	{
		AddEntity(new Debris(pos));
	}
}


void Room::ExplodeAllEntititiesExceptPlayer()
{
	for (Entities::iterator it = m_entities.begin(); it != m_entities.end(); ++it)
	{
		Entity* pEntity = *it;

		if (pEntity->IsEnabled() && (pEntity != &Player::GetPlayer()))
		{
			// Don't explode explosions
			Entity::ELifeTime eLifeTime = pEntity->GetLifeTime();
			if ((eLifeTime != Entity::eLT_Animation) && (eLifeTime != Entity::eLT_Collision))
			{
				pEntity->ForceExplode();
			}
		}
	}
}


void Room::RemoveEntities(Entity::ELifeTime eMinimumLifeTimeToRemove)
{
	for (Entities::iterator it = m_entities.begin(); it != m_entities.end(); )
	{
		Entity* pEntity = *it;

		if (pEntity->GetLifeTime() >= eMinimumLifeTimeToRemove)
		{
			delete pEntity;
			it = m_entities.erase(it);
		}
		else
		{
			++it;
		}
	}
}


void Room::AddExplosion()
{
	Vector2 pos(ZERO);
	Explosion* pExplosion = new Explosion(pos);
	
	// Adding explosion close to an obstacle, otherwise it may not look very plausible
	do
	{
		pos.Set(
			float(rand() % (Engine::eScreenWidthInPixels / pExplosion->GetWidth())),
			float(2 + rand() % (Engine::eScreenHeightInPixels / pExplosion->GetHeight() - 2)));
		pos *= float(pExplosion->GetWidth());
		pExplosion->SetPos(pos);
	} while (OverlapsObstacles(pExplosion, pos.x, pos.y) ||
			!(OverlapsObstacles(pExplosion, pos.x + 16.f, pos.y) ||
			  OverlapsObstacles(pExplosion, pos.x, pos.y + 16.f) ||
			  OverlapsObstacles(pExplosion, pos.x - 16.f, pos.y) ||
			  OverlapsObstacles(pExplosion, pos.x, pos.y - 16.f)));

	AddEntity(pExplosion);
	AddDebris(pos);
}
