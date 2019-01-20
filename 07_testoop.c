#include <stdio.h>
#include <stdlib.h>


enum ACTOR_TYPE
{
	HERO = 0,
	COIN,
	ENEMI,
	BOSS,
	MAX_ACTOR_TYPE
};

typedef struct actor
{
	int type;
	void (*update)(float deltaTime);
	void (*render)();
	void (*destroy)();
} actor_t;

#define INSTANCE_ACTOR(this) (ACTOR_THIS = this)

actor_t* ACTOR_THIS = NULL;

void hero_update(float deltaTime);
void hero_render();
void hero_destroy();

actor_t* hero_new()
{
	actor_t * a = (actor_t *)calloc(1, sizeof(actor_t));

	if (a)
	{
		a->type = HERO;
		a->update = hero_update;
		a->render = hero_render;
		a->destroy = hero_destroy;
	}

	return a;
}

void hero_update(float deltaTime)
{
	printf("hero_update\n");
}

void hero_render()
{
	printf("hero_render\n");
}

void hero_destroy()
{
	printf("hero_destroy\n");
	free((void*)ACTOR_THIS);
}

void coin_update(float deltaTime);
void coin_render();
void coin_destroy();

actor_t* coin_new()
{
	actor_t * a = (actor_t *)calloc(1, sizeof(actor_t));

	if (a)
	{
		a->type = COIN;
		a->update = coin_update;
		a->render = coin_render;
		a->destroy = coin_destroy;
	}

	return a;
}

void coin_update(float deltaTime)
{
	printf("coin_update\n");
}

void coin_render()
{
	printf("coin_render\n");
}

void coin_destroy()
{
	printf("coin_destroy\n");
	free((void*)ACTOR_THIS);
}

void enemi_update(float deltaTime);
void enemi_render();
void enemi_destroy();

actor_t* enemi_new()
{
	actor_t * a = (actor_t *)calloc(1, sizeof(actor_t));

	if (a)
	{
		a->type = ENEMI;
		a->update = enemi_update;
		a->render = enemi_render;
		a->destroy = enemi_destroy;
	}

	return a;
}

void enemi_update(float deltaTime)
{
	printf("enemi_update\n");
}

void enemi_render()
{
	printf("enemi_render\n");
}

void enemi_destroy()
{
	printf("enemi_destroy\n");
	free((void*)ACTOR_THIS);
}

void boss_update(float deltaTime);
void boss_render();
void boss_destroy();

actor_t* boss_new()
{
	actor_t * a = (actor_t *)calloc(1, sizeof(actor_t));

	if (a)
	{
		a->type = BOSS;
		a->update = boss_update;
		a->render = boss_render;
		a->destroy = boss_destroy;
	}

	return a;
}

void boss_update(float deltaTime)
{
	printf("boss_update\n");
}

void boss_render()
{
	printf("boss_render\n");
}

void boss_destroy()
{
	printf("boss_destroy\n");
	free((void*)ACTOR_THIS);
}

// actor factory
actor_t * actor_create(int type)
{
	typedef actor_t* (*actor_new_callback)();

	static actor_new_callback actor_new_array[MAX_ACTOR_TYPE] =
	{
		hero_new,
		coin_new,
		enemi_new,
		boss_new
	};

	return actor_new_array[type]();
}

int main()
{
	actor_t * hero = actor_create(0);
	actor_t * list[8];

	INSTANCE_ACTOR(hero)->update(1.0f/60.0f);
	INSTANCE_ACTOR(hero)->render();
	INSTANCE_ACTOR(hero)->destroy();

	return 0;
}
