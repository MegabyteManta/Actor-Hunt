#pragma once

#include <map>
#include <string>
#include <vector>
#include <Windows.h>

class Actor;
class World;

typedef unsigned int ComponentTypeID;

class Vector2
{
public:
	float x, y;

	Vector2() : x(0), y(0) {}
	Vector2(float ix, float iy) : x(ix), y(iy) {}

	Vector2& operator= (const Vector2& rhs) { x = rhs.x; y = rhs.y; return *this; }
	bool operator== (const Vector2& rhs) const { return x == rhs.x && y == rhs.y; }

	// add or subtract the given vector to this
	void Add(const Vector2& rhs) { x += rhs.x; y += rhs.y; }
	void Sub(const Vector2& rhs) { x -= rhs.x; y -= rhs.y; }
};

/**
* Represents the position of a pawn in the world and which direction it's facing.
*/
class Frame
{
public:
	Vector2 pos;
	Vector2 facing;
};


class Component
{
public:

	virtual ~Component();

	/** This gets called on every simulation frame. */
	virtual void Tick() = 0;

	/** Set the actor this component is attached to. */
	virtual void SetActor(Actor* actor);

	/** A unique ID representing this component class type.
	   values 0-999 are reserves for Engine components. */
	virtual ComponentTypeID GetComponentTypeID() const = 0;

protected:
	Actor* _actor = nullptr;
};

/**
* Singleton class that manages buffered input.
* This uses Windows virtual keycode: https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
*/
class InputManager
{
public:
	static InputManager& GetInstance();

	/** Query for input */
	void CaptureInput();

	/** Returns true if the key has been pressed for the first time this frame.
	    if you hold it down, this will return true on the first frame and false on subsequent frames. */
	bool IsKeyFirstPressed(int key) const;

	/** Returns true if the key is currently being pressed. */
	bool IsKeyPressed(int key) const;

	/** returns true if any key has been pressed for the first time this frame. */
	bool IsAnyKeyFirstPressed() const;

private:

	InputManager();
	~InputManager();

	static const int NUM_KEYS = 256;
	SHORT* _keyMap;
	bool _isAnyKeyFirstPressed;

	const SHORT MASK_KEYDOWN = static_cast<SHORT>(0x0001);
	const SHORT MASK_KEYPRESSED = static_cast<SHORT>(0x8000);
};


/**
* Moves the actor using WASD controls.
*/
class MoveComponent : public Component
{
	virtual void Tick() override;
	virtual ComponentTypeID GetComponentTypeID() const override { return 1; }
};

class Actor
{
public:

	Actor();
	virtual ~Actor() = 0;

	/** This is gets called on every simulation frame. */
	virtual void Tick();

	/** Attach a component to this actor. */
	void AddComponent(Component* component);

	/** Access an attached component. */
	Component* GetComponent(ComponentTypeID ct) const;

	/** General key->value pairs that can be used for game defined properties. */
	void SetAttribute(std::string name, int val);
	bool TryGetAttribute(std::string name, int& val) const;

	/** The world that this Actor is in. */
	World * GetWorld() const;
	void SetWorld(World* world);

private:
	std::vector<Component*> _components;
	std::map<std::string, int> _attributes;
	World * _world = nullptr;
};


/**
*An actor that has a physical presence in the world.
*/
class Pawn : public Actor
{
public:
	Pawn();
	/** Move the pawn in the given directon*/
	virtual void ApplyMovement(Vector2 movement);
	virtual void Tick() override;

	virtual Frame GetFrame();

protected:
	Vector2 _velocity;
	Frame _frame;
};


/**
* World is composed of actors with movement components laid out on a grid
*/
class World : Actor
{
public:
	World();
	~World() override;

	/** Calls tick on every actor. */
	virtual void Tick() override;

	template<typename TActorType>
	TActorType* CreateActor();

	/** Return all the objects that descend from the Pawn class that are at this given location */
	template <class TPawnType>
	std::vector<TPawnType*> FindPawnsAtLocation(float x, float y);

	const std::vector<Actor*> & GetAllActors() const { return _actors; }

protected:
	std::vector< Actor* > _actors;
};


template <typename TActorType>
TActorType* World::CreateActor()
{
	TActorType* actor = new TActorType();
	_actors.push_back(actor);
	actor->SetWorld(this);
	return actor;
}


template <class TPawnType>
std::vector<TPawnType*> World::FindPawnsAtLocation(float x, float y)
{
	std::vector<TPawnType*> pawns;
	for (unsigned int i = 0; i < _actors.size(); ++i)
	{
		Actor* actor = _actors[i];
		TPawnType* castedPawn = dynamic_cast<TPawnType*>(actor);
		if (castedPawn &&
			castedPawn->GetFrame().pos.x == x && castedPawn->GetFrame().pos.y == y)
		{
			pawns.push_back(castedPawn);
		}
	}

	return pawns;
}