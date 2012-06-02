
#pragma once

#ifndef _WIN32
	#include "BlockID.h"
#else
	enum ENUM_ITEM_ID;
#endif

#define MAX_PLAYERS 65535

#include "cSimulatorManager.h"
#include "MersenneTwister.h"
#include "cChunkMap.h"
#include "WorldStorage.h"
#include "cChunkGenerator.h"
#include "Vector3i.h"
#include "Vector3f.h"
#include "ChunkSender.h"
#include "Defines.h"
#include "LightingThread.h"





class cPacket;
class cRedstone;
class cFireSimulator;
class cWaterSimulator;
class cLavaSimulator;
class cSandSimulator;
class cRedstoneSimulator;
class cItem;
class cPlayer;
class cClientHandle;
class cEntity;
class cBlockEntity;
class cWorldGenerator;  // The generator that actually generates the chunks for a single world
class cChunkGenerator;  // The thread responsible for generating chunks
typedef std::list< cPlayer * > cPlayerList;
typedef cItemCallback<cPlayer> cPlayerListCallback;
typedef cItemCallback<cEntity> cEntityCallback;





class cWorld													//tolua_export
{																//tolua_export
public:

	OBSOLETE static cWorld* GetWorld();

	// Return time in seconds
	inline static float GetTime()													//tolua_export
	{
		return m_Time;
	}
	long long GetWorldTime(void) const { return m_WorldTime; }						//tolua_export

	eGameMode GetGameMode(void) const { return m_GameMode; }						//tolua_export

	void SetWorldTime(long long a_WorldTime) { m_WorldTime = a_WorldTime; }			//tolua_export

	int GetHeight( int a_X, int a_Z );												//tolua_export

	void Broadcast( const cPacket & a_Packet, cClientHandle* a_Exclude = 0 );
	void BroadcastToChunk(int a_ChunkX, int a_ChunkY, int a_ChunkZ, const cPacket & a_Packet, cClientHandle * a_Exclude = NULL);
	void BroadcastToChunkOfBlock(int a_X, int a_Y, int a_Z, cPacket * a_Packet, cClientHandle * a_Exclude = NULL);
	
	void MarkChunkDirty (int a_ChunkX, int a_ChunkY, int a_ChunkZ);
	void MarkChunkSaving(int a_ChunkX, int a_ChunkY, int a_ChunkZ);
	void MarkChunkSaved (int a_ChunkX, int a_ChunkY, int a_ChunkZ);
	
	/** Sets the chunk data as either loaded from the storage or generated.
	a_BlockLight and a_BlockSkyLight are optional, if not present, chunk will be marked as unlighted.
	a_BiomeMap is optional, if not present, biomes will be calculated by the generator
	a_HeightMap is optional, if not present, will be calculated.
	If a_MarkDirty is set, the chunk is set as dirty (used after generating)
	*/
	void SetChunkData(
		int a_ChunkX, int a_ChunkY, int a_ChunkZ, 
		const BLOCKTYPE *  a_BlockTypes,
		const NIBBLETYPE * a_BlockMeta,
		const NIBBLETYPE * a_BlockLight,
		const NIBBLETYPE * a_BlockSkyLight,
		const cChunkDef::HeightMap * a_HeightMap,
		const cChunkDef::BiomeMap  * a_BiomeMap,
		cEntityList & a_Entities,
		cBlockEntityList & a_BlockEntities,
		bool a_MarkDirty
	);
	
	void ChunkLighted(
		int a_ChunkX, int a_ChunkZ,
		const cChunkDef::BlockNibbles & a_BlockLight,
		const cChunkDef::BlockNibbles & a_SkyLight
	);
	
	bool GetChunkData      (int a_ChunkX, int a_ChunkY, int a_ChunkZ, cChunkDataCallback & a_Callback);
	
	/// Gets the chunk's blocks, only the block types
	bool GetChunkBlockTypes(int a_ChunkX, int a_ChunkY, int a_ChunkZ, BLOCKTYPE * a_BlockTypes);
	
	/// Gets the chunk's blockdata, the entire 4 arrays (Types, Meta, Light, SkyLight)
	bool GetChunkBlockData (int a_ChunkX, int a_ChunkY, int a_ChunkZ, BLOCKTYPE * a_BlockData);
	
	bool IsChunkValid      (int a_ChunkX, int a_ChunkY, int a_ChunkZ) const;
	bool HasChunkAnyClients(int a_ChunkX, int a_ChunkY, int a_ChunkZ) const;
	
	void UnloadUnusedChunks(void);    // tolua_export
	
	void CollectPickupsByPlayer(cPlayer * a_Player);

	// MOTD
	const AString & GetDescription(void) const {return m_Description; }	// FIXME: This should not be in cWorld

	// Max Players
	unsigned int GetMaxPlayers(void) const {return m_MaxPlayers; }					//tolua_export
	void SetMaxPlayers(int iMax);													//tolua_export

	void AddPlayer( cPlayer* a_Player );
	void RemovePlayer( cPlayer* a_Player );

 	bool ForEachPlayer(cPlayerListCallback & a_Callback);  // >> EXPORTED IN MANUALBINDINGS <<
	
	unsigned int GetNumPlayers();													//tolua_export
	
	// TODO: This interface is dangerous - rewrite to DoWithPlayer(playername, action)
	cPlayer * GetPlayer( const char * a_PlayerName );								//tolua_export
	
	// TODO: This interface is dangerous - rewrite to DoWithClosestPlayer(pos, sight, action)
	cPlayer * FindClosestPlayer(const Vector3f & a_Pos, float a_SightLimit);
	
	void SendPlayerList(cPlayer * a_DestPlayer);  // Sends playerlist to the player

	void AddEntity( cEntity* a_Entity );
	
	/// Add an entity to the chunk specified; broadcasts the a_SpawnPacket to all clients of that chunk
	void AddEntityToChunk(cEntity * a_Entity, int a_ChunkX, int a_ChunkY, int a_ChunkZ, cPacket * a_SpawnPacket);
	
	/// Removes the entity from the chunk specified
	void RemoveEntityFromChunk(cEntity * a_Entity, int a_ChunkX, int a_ChunkY, int a_ChunkZ);
	
	/// Moves the entity from its current chunk to the new chunk specified
	void MoveEntityToChunk(cEntity * a_Entity, int a_ChunkX, int a_ChunkY, int a_ChunkZ);

	/// Compares clients of two chunks, calls the callback accordingly
	void CompareChunkClients(int a_ChunkX1, int a_ChunkY1, int a_ChunkZ1, int a_ChunkX2, int a_ChunkY2, int a_ChunkZ2, cClientDiffCallback & a_Callback);
	
	/// Adds client to a chunk, if not already present; returns true if added, false if present
	bool AddChunkClient(int a_ChunkX, int a_ChunkY, int a_ChunkZ, cClientHandle * a_Client);
	
	/// Removes client from the chunk specified
	void RemoveChunkClient(int a_ChunkX, int a_ChunkY, int a_ChunkZ, cClientHandle * a_Client);
	
	/// Removes the client from all chunks it is present in
	void RemoveClientFromChunks(cClientHandle * a_Client);
	
	/// Sends the chunk to the client specified, if the chunk is valid. If not valid, the request is postponed (ChunkSender will send that chunk when it becomes valid+lighted)
	void SendChunkTo(int a_ChunkX, int a_ChunkY, int a_ChunkZ, cClientHandle * a_Client);
	
	/// Removes client from ChunkSender's queue of chunks to be sent
	void RemoveClientFromChunkSender(cClientHandle * a_Client);
	
	/// Touches the chunk, causing it to be loaded or generated
	void TouchChunk(int a_ChunkX, int a_ChunkY, int a_ChunkZ);
	
	/// Loads the chunk, if not already loaded. Doesn't generate. Returns true if chunk valid (even if already loaded before)
	bool LoadChunk(int a_ChunkX, int a_ChunkY, int a_ChunkZ);
	
	/// Loads the chunks specified. Doesn't report failure, other than chunks being !IsValid()
	void LoadChunks(const cChunkCoordsList & a_Chunks);
	
	/// Marks the chunk as failed-to-load:
	void ChunkLoadFailed(int a_ChunkX, int a_ChunkY, int a_ChunkZ);
	
	void UpdateSign(int a_X, int a_Y, int a_Z, const AString & a_Line1, const AString & a_Line2, const AString & a_Line3, const AString & a_Line4);	//tolua_export

	/// Marks (a_Stay == true) or unmarks (a_Stay == false) chunks as non-unloadable. To be used only by cChunkStay!
	void ChunksStay(const cChunkCoordsList & a_Chunks, bool a_Stay = true);
	
	/// Regenerate the given chunk:
	void RegenerateChunk(int a_ChunkX, int a_ChunkZ);													//tolua_export
	
	/// Generates the given chunk, if not already generated
	void GenerateChunk(int a_ChunkX, int a_ChunkZ);													//tolua_export
	
	/// Queues a chunk for lighting; a_Callback is called after the chunk is lighted
	void QueueLightChunk(int a_ChunkX, int a_ChunkZ, cChunkCoordCallback * a_Callback = NULL);
	
	bool IsChunkLighted(int a_ChunkX, int a_ChunkZ);

	// TODO: Export to Lua
	bool DoWithEntity( int a_UniqueID, cEntityCallback & a_Callback );

	void SetBlock( int a_X, int a_Y, int a_Z, char a_BlockType, char a_BlockMeta );						//tolua_export
	void FastSetBlock( int a_X, int a_Y, int a_Z, char a_BlockType, char a_BlockMeta );					//tolua_export
	char GetBlock( int a_X, int a_Y, int a_Z );															//tolua_export
	char GetBlock( const Vector3i & a_Pos ) { return GetBlock( a_Pos.x, a_Pos.y, a_Pos.z ); }			//tolua_export
	char GetBlockMeta( int a_X, int a_Y, int a_Z );														//tolua_export
	char GetBlockMeta( const Vector3i & a_Pos ) { return GetBlockMeta( a_Pos.x, a_Pos.y, a_Pos.z ); }	//tolua_export
	void SetBlockMeta( int a_X, int a_Y, int a_Z, char a_MetaData );									//tolua_export
	void SetBlockMeta( const Vector3i & a_Pos, char a_MetaData ) { SetBlockMeta( a_Pos.x, a_Pos.y, a_Pos.z, a_MetaData ); } //tolua_export
	char GetBlockSkyLight( int a_X, int a_Y, int a_Z );													//tolua_export
	// TODO: char GetBlockActualLight(int a_BlockX, int a_BlockY, int a_BlockZ);  // tolua_export
	
	/// Replaces world blocks with a_Blocks, if they are of type a_FilterBlockType
	void ReplaceBlocks(const sSetBlockVector & a_Blocks, BLOCKTYPE a_FilterBlockType);
	
	/// Retrieves block types of the specified blocks. If a chunk is not loaded, doesn't modify the block. Returns true if all blocks were read.
	bool GetBlocks(sSetBlockVector & a_Blocks, bool a_ContinueOnFailure);
	
	bool DigBlock( int a_X, int a_Y, int a_Z, cItem & a_PickupItem );									//tolua_export
	void SendBlockTo( int a_X, int a_Y, int a_Z, cPlayer* a_Player );									//tolua_export

	const double & GetSpawnX() { return m_SpawnX; }														//tolua_export
	const double & GetSpawnY();																			//tolua_export
	const double & GetSpawnZ() { return m_SpawnZ; }														//tolua_export

	inline cSimulatorManager *GetSimulatorManager() { return m_SimulatorManager; }
	inline cWaterSimulator *GetWaterSimulator() { return m_WaterSimulator; }
	inline cLavaSimulator *GetLavaSimulator() { return m_LavaSimulator; }

	// TODO: This interface is dangerous! Export as a set of specific action functions for Lua: GetChestItem, GetFurnaceItem, SetFurnaceItem, SetSignLines etc.
	// _X 2012_02_21: This function always returns NULL
	OBSOLETE cBlockEntity * GetBlockEntity( int a_X, int a_Y, int a_Z );						//tolua_export
	
	/// a_Player is using block entity at [x, y, z], handle that:
	void UseBlockEntity(cPlayer * a_Player, int a_X, int a_Y, int a_Z) {m_ChunkMap->UseBlockEntity(a_Player, a_X, a_Y, a_Z); }

	void GrowTree           (int a_BlockX, int a_BlockY, int a_BlockZ);                           // tolua_export
	void GrowTreeFromSapling(int a_BlockX, int a_BlockY, int a_BlockZ, char a_SaplingMeta);       // tolua_export
	void GrowTreeByBiome    (int a_BlockX, int a_BlockY, int a_BlockZ);                           // tolua_export
	
	void GrowTreeImage(const sSetBlockVector & a_Blocks);

	int  GetBiomeAt (int a_BlockX, int a_BlockZ);   // tolua_export

	const AString & GetName(void) const { return m_WorldName; }									//tolua_export
	const AString & GetIniFileName(void) const {return m_IniFileName; }

	inline static void AbsoluteToRelative( int & a_X, int & a_Y, int & a_Z, int & a_ChunkX, int & a_ChunkY, int & a_ChunkZ )
	{
		// TODO: Use floor() instead of weird if statements
		// Also fix Y
		a_ChunkX = a_X/cChunkDef::Width;
		if(a_X < 0 && a_X % cChunkDef::Width != 0) a_ChunkX--;
		a_ChunkY = 0;
		a_ChunkZ = a_Z/cChunkDef::Width;
		if(a_Z < 0 && a_Z % cChunkDef::Width != 0) a_ChunkZ--;

		a_X = a_X - a_ChunkX*cChunkDef::Width;
		a_Y = a_Y - a_ChunkY*cChunkDef::Height;
		a_Z = a_Z - a_ChunkZ*cChunkDef::Width;
	}
	
	inline static void BlockToChunk( int a_X, int a_Y, int a_Z, int & a_ChunkX, int & a_ChunkY, int & a_ChunkZ )
	{
		// TODO: Use floor() instead of weird if statements
		// Also fix Y
		(void)a_Y; // not unused anymore
		a_ChunkX = a_X/cChunkDef::Width;
		if(a_X < 0 && a_X % cChunkDef::Width != 0) a_ChunkX--;
		a_ChunkY = 0;
		a_ChunkZ = a_Z/cChunkDef::Width;
		if(a_Z < 0 && a_Z % cChunkDef::Width != 0) a_ChunkZ--;
	}

	void SaveAllChunks(void);			//tolua_export

	/// Returns the number of chunks loaded	
	int GetNumChunks() const;		//tolua_export

	/// Returns the number of chunks loaded and dirty, and in the lighting queue
	void GetChunkStats(int & a_NumValid, int & a_NumDirty, int & a_NumInLightingQueue);

	// Various queues length queries (cannot be const, they lock their CS):
	inline int GetGeneratorQueueLength  (void) { return m_Generator.GetQueueLength();   }    // tolua_export
	inline int GetLightingQueueLength   (void) { return m_Lighting.GetQueueLength();    }    // tolua_export
	inline int GetStorageLoadQueueLength(void) { return m_Storage.GetLoadQueueLength(); }    // tolua_export
	inline int GetStorageSaveQueueLength(void) { return m_Storage.GetSaveQueueLength(); }    // tolua_export

	void Tick(float a_Dt);

	void InitializeSpawn();

	void CastThunderbolt (int a_X, int a_Y, int a_Z);						//tolua_export
	void SetWeather ( eWeather a_Weather );									//tolua_export
	eWeather GetWeather() { return m_Weather; };							//tolua_export

	cChunkGenerator & GetGenerator(void) { return m_Generator; }
	cWorldStorage &   GetStorage  (void) { return m_Storage; }
	cChunkMap *       GetChunkMap (void) { return m_ChunkMap; }
	
	bool IsPlacingItemLegal(Int16 a_ItemType, int a_BlockX, int a_BlockY, int a_BlockZ);
	
	/// Sets the blockticking to start at the specified block. Only one blocktick per chunk may be set, second call overwrites the first call
	void SetNextBlockTick(int a_BlockX, int a_BlockY, int a_BlockZ);  // tolua_export
	
private:

	friend class cRoot;

	// This random generator is to be used only in the Tick() method, and thus only in the World-Tick-thread (MTRand is not exactly thread-safe)
	MTRand m_TickRand;

	double m_SpawnX;
	double m_SpawnY;
	double m_SpawnZ;

	float m_LastUnload;
	float m_LastSave;
	static float m_Time;	// Time in seconds
	long long m_WorldTime; // Time in seconds*20, this is sent to clients (is wrapped)
	unsigned long long CurrentTick;
	eGameMode m_GameMode;
	float m_WorldTimeFraction; // When this > 1.f m_WorldTime is incremented by 20

	// The cRedstone class simulates redstone and needs access to m_RSList
	friend class cRedstone;
	std::vector<int> m_RSList;

	cSimulatorManager *  m_SimulatorManager;
	cSandSimulator *     m_SandSimulator;
	cWaterSimulator *    m_WaterSimulator;
	cLavaSimulator *     m_LavaSimulator;
	cFireSimulator *     m_FireSimulator;
	cRedstoneSimulator * m_RedstoneSimulator;
	
	cCriticalSection m_CSClients;
	cCriticalSection m_CSEntities;
	cCriticalSection m_CSPlayers;

	cWorldStorage     m_Storage;
	
	AString m_Description;
	
	unsigned int m_MaxPlayers;

	cChunkMap * m_ChunkMap;

	bool m_bAnimals;
	float m_SpawnMonsterTime;
	float m_SpawnMonsterRate;

	eWeather m_Weather;
	
	cEntityList       m_RemoveEntityQueue;
	cEntityList       m_AllEntities;
	cClientHandleList m_Clients;
	cPlayerList       m_Players;

	cCriticalSection m_CSFastSetBlock;
	sSetBlockList    m_FastSetBlockQueue;

	cChunkGenerator  m_Generator;
	
	cChunkSender     m_ChunkSender;
	cLightingThread  m_Lighting;

	AString m_WorldName;
	AString m_IniFileName;
	
	cWorld(const AString & a_WorldName);
	~cWorld();

	void TickWeather(float a_Dt);  // Handles weather each tick
	void TickSpawnMobs(float a_Dt);  // Handles mob spawning each tick
	
	void RemoveEntity( cEntity * a_Entity );
}; //tolua_export




