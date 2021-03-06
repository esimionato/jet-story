#include "Entity.h"
#include "Maze.h"
#include "Player.h"
#include "Projectiles.h"


Entity::Entity(const Vector2& vInitialPos) :
	m_eLifeTime(eLT_Session),
	m_vInitialPos(vInitialPos),
	m_vel(ZERO),
	m_bEnabled(true),
	m_timer(0),
	m_pPlatform(nullptr)
{
	SetPos(m_vInitialPos);
}


void Entity::Reset()
{
	Sprite::Reset();

	SetPos(m_vInitialPos);
	m_vel.Zero();
	m_bEnabled = true;
	m_timer = 0;
}


void Entity::Animate()
{
	int oldFrame = m_nAnimationFrame;
	Sprite::Animate();

	if (m_eLifeTime == eLT_Animation)
	{
		if ((oldFrame > 0) && (m_nAnimationFrame == 0))
		{
			SetGarbage();
		}
	}
}


void Entity::Update()
{
	if (m_timer > 0)
	{
		if (--m_timer <= 0)
		{
			OnTimer();
		}
	}
}


void Entity::Explode()
{
	m_bEnabled = false;
	Player::GetPlayer().AddScore(GetScore());
	
	Maze& maze = Maze::GetMaze();
	
	for (int row = 0; row < m_nHeight; row += 16)
	{
		for (int col = 0; col < GetWidth(); col += 16)
		{
			Vector2 pos = m_pos;
			pos.x += col;
			pos.y += row;
			maze.AddEntity(new Explosion(pos));
			maze.AddDebrisFor(pos + Vector2(4, 4));
		}
	}

	if (m_pPlatform)
	{
		m_pPlatform->Explode();
	}
}
