#pragma once

#include "Engine.h"
#include <unordered_map>

using namespace std;

// The directions a pawn can face.
enum PawnDirections { none, left, right, up, down };

// Base class for all pawns in the game
class ActorHuntPawn : public Pawn
{
public:
	ActorHuntPawn();

	/**
	* Called once per world tick. Calls ClearMessage()
	*/
	virtual void Tick() override;

	/**
	* Gets the pawn's sigil. An undefined sigil returns a default char
	* @return char : the pawn's sigil
	*/
	char GetSigil();

	/**
	* Sets the frame of the pawn. Used for moving pawns without changing direction
	* @param Frame frame : the frame to set the pawn to
	*/
	void SetFrame(Frame frame);

	/**
	* Modifies an attribute. Differs from SetAttribute by adding val to attribute instead of setting it to val
	* Also does not add a new attribute if the attribute is not found.
	* @param string name : the name of the attribute to modify
	* @param val : the value to add to the attribute
	* @return bool : if attribute was successfully modified
	*/
	virtual bool ModifyAttribute(string name, int val);

	/**
	* Called when another pawn overlaps this pawn
	* @param Pawn* pawn : the overlapped pawn
	*/
	virtual void OnOverlappedPawn(Pawn* pawn) {};

	/**
	* Add message to the list of messages
	* @param string message : the message to add
	* @param bool toFront = false : add the message to the front of the list?
	*/
	void AddMessage(string message, bool toFront = false);

	/**
	* Returns this pawn's messages
	* @return const list<string>& : the messages
	*/
	const list<string>& GetMessages() const { return _messages; };

	/**
	* Clears this pawn's messages
	*/
	void ClearMessages();

protected:
	// The characters representing the pawn and which way it's facing
	unordered_map<PawnDirections, char> _sigils;

	// The messages that this pawn should print for this tick
	list<string> _messages;

	// Rep invariants:
	//   none
};

// The player's pawn
class Player : public ActorHuntPawn
{
public:
	Player();

	/**
	* Called once per world tick. Handles manual input for damage and heal
	*/
	virtual void Tick() override;

	/**
	* Called when a pawn overlaps this pawn. Handles damage, heal, and score changes
	*/
	virtual void OnOverlappedPawn(Pawn* pawn) override;

protected:
	// Initial hp amount
	int _initHp;

	// Initial score amount
	int _initScore;

	// Rep invariants:
	//   none
};

// A damaging pawn
class Damager : public ActorHuntPawn
{
public:
	Damager();

protected:
	// The amount of damage this pawn can do
	int _damageAmt;

	// The points this pawn will give
	int _points;

	// Rep invariants:
	//   none
};

// A healing pawn
class Healer : public ActorHuntPawn
{
public:
	Healer();

protected:
	// The amount of health this pawn can give
	int _healAmt;

	// The points this pawn will give
	int _points;

	// Rep invariants:
	//   none
};