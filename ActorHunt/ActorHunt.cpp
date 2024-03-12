#include "ActorHunt.h"
#include <sstream>

using namespace std;

ActorHuntPawn::ActorHuntPawn()
{
	_sigils = {
		{PawnDirections::none, '?'},
		{PawnDirections::left, '?'},
		{PawnDirections::right, '?'},
		{PawnDirections::up, '?'},
		{PawnDirections::down, '?'}
	};
}

// clear old messages at the start of every tick to be prepared
// for new messages for this tick
void ActorHuntPawn::Tick()
{
	Pawn::Tick();
	ClearMessages();
}

char ActorHuntPawn::GetSigil()
{
	char sigil = _sigils[PawnDirections::none];
	if (_frame.facing.x > 0)
	{
		sigil = _sigils[PawnDirections::left];
	}
	else if (_frame.facing.x < 0)
	{
		sigil = _sigils[PawnDirections::right];
	}
	if (_frame.facing.y > 0)
	{
		sigil = _sigils[PawnDirections::up];
	}
	else if (_frame.facing.y < 0)
	{
		sigil = _sigils[PawnDirections::down];
	}

	return sigil;
}

// set a pawn's position and facing direction manually
// without changing the pawn's sigil
void ActorHuntPawn::SetFrame(Frame frame)
{
	_frame = frame;
}

// modify the value of attribute "name" by value val
// also add messages for the modified attribute
bool ActorHuntPawn::ModifyAttribute(string name, int val)
{
	int prevVal = 0;
	if (TryGetAttribute(name, prevVal))
	{
		SetAttribute(name, prevVal + val);

		if (name == "hp")
		{
			int hpDiff = val;
			if (hpDiff <= -20)
			{
				AddMessage(" player took a critical hit!");
			}
			if (hpDiff < 0)
			{
				ostringstream message;
				message << " player just took " << -1 * hpDiff << " damage.";
				AddMessage(message.str());
			}
			if (hpDiff > 0)
			{
				ostringstream message;
				message << " player just gained " << hpDiff << " health.";
				AddMessage(message.str());
			}
		}
		return true;
	}
	return false;
}

// add a message either to the front or back of the list of messages
void ActorHuntPawn::AddMessage(std::string message, bool toFront)
{
	if (toFront)
	{
		_messages.emplace_front(message);
	}
	else
	{
		_messages.emplace_back(message);
	}

}
void ActorHuntPawn::ClearMessages()
{
	_messages.clear();
}

Player::Player()
: _initHp(100), _initScore(0)
{
	SetAttribute("hp", _initHp);
	SetAttribute("score", _initScore);

	_sigils = {
			{PawnDirections::none, 'o'},
			{PawnDirections::left, '<'},
			{PawnDirections::right, '>'},
			{PawnDirections::up, '^'},
			{PawnDirections::down, 'V'}
	};
}

// handle input for manually changing attributes
void Player::Tick()
{
	ActorHuntPawn::Tick();

	InputManager& input = InputManager::GetInstance();

	// simulate damage
	if (input.IsKeyFirstPressed('1'))
	{
		ModifyAttribute("hp", -10);
	}
	if (input.IsKeyFirstPressed('2'))
	{
		ModifyAttribute("hp", -20);
	}

	// simulate heal
	if (input.IsKeyFirstPressed('3'))
	{
		ModifyAttribute("hp", 10);
	}
	if (input.IsKeyFirstPressed('4'))
	{
		ModifyAttribute("hp", 20);
	}

	// simulate points
	if (input.IsKeyFirstPressed('5'))
	{
		ModifyAttribute("score", 10);
	}
	if (input.IsKeyFirstPressed('6'))
	{
		ModifyAttribute("score", 20);
	}

}

// Handle damage, health, and points given by other pawns.
void Player::OnOverlappedPawn(Pawn* pawn)
{
	int damage = 0;
	pawn->TryGetAttribute("damage", damage);
	int heal = 0;
	pawn->TryGetAttribute("heal", heal);

	int hp = 0;
	ModifyAttribute("hp", heal);
	ModifyAttribute("hp", -1*damage);

	int points = 0;
	pawn->TryGetAttribute("points", points);

	int score = 0;
	ModifyAttribute("score", points);
}

Damager::Damager()
	: _damageAmt(20), _points(10)
{
	SetAttribute("points", _points);
	SetAttribute("damage", _damageAmt);

	_sigils = {
		{PawnDirections::none, '-'},
		{PawnDirections::left, '-'},
		{PawnDirections::right, '-'},
		{PawnDirections::up, '-'},
		{PawnDirections::down, '-'}
	};
}

Healer::Healer()
	: _healAmt(5), _points(5)
{
	SetAttribute("points", _points);
	SetAttribute("heal", _healAmt);

	_sigils = {
		{PawnDirections::none, '+'},
		{PawnDirections::left, '+'},
		{PawnDirections::right, '+'},
		{PawnDirections::up, '+'},
		{PawnDirections::down, '+'}
	};
}
