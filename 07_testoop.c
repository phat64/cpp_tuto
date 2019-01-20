#include <stdio.h>
#include <stdlib.h>

// Code exemple for OOP in C but imcomplete (need inherence)

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

// ---------------------- HERO ----------------------------

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

// ---------------------- COIN ----------------------------

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

// ---------------------- ENEMI ----------------------------

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

// ---------------------- BOSS ----------------------------

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

// ----------------------------------------------------------

// actor factory - optimized version
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

// -----------------------------------------------------------

void update_all_actors(actor_t* list[], int n, float deltaTime)
{
	int i;

	for (i = 0; i < n; i++)
	{
		INSTANCE_ACTOR(list[i])->update(deltaTime);
	}
}

void render_all_actors(actor_t* list[], int n)
{
	int i;

	for (i = 0; i < n; i++)
	{
		INSTANCE_ACTOR(list[i])->render();
	}

}

void destroy_all_actors(actor_t* list[], int n)
{
	int i;

	for (i = 0; i < n; i++)
	{
		INSTANCE_ACTOR(list[i])->destroy();
	}

}

// -----------------------------------------------------------

int main()
{
	actor_t * list[7];

	//INSTANCE_ACTOR(hero)->update(1.0f/60.0f);
	//INSTANCE_ACTOR(hero)->render();
	//INSTANCE_ACTOR(hero)->destroy();

	list[0] = actor_create(HERO);
	list[1] = actor_create(COIN);
	list[2] = actor_create(COIN);
	list[3] = actor_create(COIN);
	list[4] = actor_create(ENEMI);
	list[5] = actor_create(ENEMI);
	list[6] = actor_create(BOSS);

	update_all_actors(list, 7, 1.0f / 60.0f);
	render_all_actors(list, 7);
	destroy_all_actors(list, 7);

	return 0;
}

/*
output :
hero_update
coin_update
coin_update
coin_update
enemi_update
enemi_update
boss_update
hero_render
coin_render
coin_render
coin_render
enemi_render
enemi_render
boss_render
hero_destroy
coin_destroy
coin_destroy
coin_destroy
enemi_destroy
enemi_destroy
boss_destroy
*/
