#include <stdio.h>
#include <stdlib.h>


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
		a->type = 0;
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


// actor factory
actor_t * actor_create(int type)
{
	return hero_new();
}

int main()
{
	actor_t * hero = actor_create(0);

	INSTANCE_ACTOR(hero)->update(1.0f/60.0f);
	INSTANCE_ACTOR(hero)->render();
	INSTANCE_ACTOR(hero)->destroy();

	return 0;
}
