#include "Engine.h"

using namespace std;

Component::~Component()
{
}

void Component::SetActor(Actor* actor)
{
	_actor = actor;
}

void MoveComponent::Tick()
{
	Pawn* pawn = dynamic_cast<Pawn*>(_actor);
	InputManager& input = InputManager::GetInstance();

	// apply movement to pawn based on player input
	if (pawn)
	{
		Vector2 movement;
		if (input.IsKeyFirstPressed('W'))
		{
			movement.y += 1.0f;
		}
		if (input.IsKeyFirstPressed('A'))
		{
			movement.x += 1.0f;
		}
		if (input.IsKeyFirstPressed('S'))
		{
			movement.y += -1.0f;
		}
		if (input.IsKeyFirstPressed('D'))
		{
			movement.x += -1.0f;
		}

		pawn->ApplyMovement(movement);
	}
}

Actor::Actor()
{}

Actor::~Actor()
{
	for (Component* component : _components)
	{
		delete component;
	}
}

void Actor::AddComponent(Component* component)
{
	component->SetActor(this);
	_components.push_back(component);
}

Component* Actor::GetComponent(ComponentTypeID ct) const
{
	for (Component* component : _components)
	{
		if (component->GetComponentTypeID() == ct)
		{
			return component;
		}
	}

	return nullptr;
}

void Actor::Tick()
{
	for (Component* component : _components)
	{
		component->Tick();
	}
}

void Actor::SetAttribute(string name, int val)
{
	auto iter = _attributes.find(name);
	if (iter == _attributes.end())
	{
		_attributes.insert(std::pair<string, int>(name, val));
	}
	else
	{
		iter->second = val;
	}
}

bool Actor::TryGetAttribute(string name, int& val) const
{
	auto iter = _attributes.find(name);
	if (iter == _attributes.end())
	{
		return false;
	}
	val = iter->second;
	return true;
}

World* Actor::GetWorld() const
{
	return _world;
}

void Actor::SetWorld(World* world)
{
	_world = world;
}


Pawn::Pawn()
{}

void Pawn::ApplyMovement(Vector2 movement)
{
	_velocity.Add(movement);
}

void Pawn::Tick()
{
	Actor::Tick();

	// update the frame if pawn has non-zero velocity
	if (_velocity.x != 0 || _velocity.y != 0)
	{
		_frame.pos.Add(_velocity);
		_frame.facing = _velocity;
	}

	// zero out velocity for use during the next tick
	memset(&_velocity, 0, sizeof(_velocity));
}

Frame Pawn::GetFrame()
{
	return _frame;
}

InputManager::InputManager()
	: _isAnyKeyFirstPressed(false)
{
	_keyMap = new SHORT[NUM_KEYS];
}

InputManager::~InputManager()
{
	delete[] _keyMap;
}

InputManager& InputManager::GetInstance()
{
	static InputManager im;
	return im;
}

void InputManager::CaptureInput()
{
	_isAnyKeyFirstPressed = false;
	for (int key = 0; key < NUM_KEYS; ++key)
	{
		SHORT curState = GetAsyncKeyState(key);
		SHORT prevState = _keyMap[key];

		curState &= ~MASK_KEYDOWN;
		if (prevState == 0 &&
			(curState & MASK_KEYPRESSED) == MASK_KEYPRESSED)
		{
			curState |= MASK_KEYDOWN;
			_isAnyKeyFirstPressed = true;
		}

		_keyMap[key] = curState;
	}
}

bool InputManager::IsKeyFirstPressed(int key) const
{
	return (_keyMap[key] & MASK_KEYDOWN) == MASK_KEYDOWN;
}

bool InputManager::IsKeyPressed(int key) const
{
	return (_keyMap[key] & MASK_KEYPRESSED) == MASK_KEYPRESSED;
}

bool InputManager::IsAnyKeyFirstPressed() const
{
	return _isAnyKeyFirstPressed;
}

World::World()
{
}

World::~World()
{
	for (Actor* actor : _actors)
	{
		delete actor;
	}
}

void World::Tick()
{
	for (Actor* actor : _actors)
	{
		actor->Tick();
	}
}

