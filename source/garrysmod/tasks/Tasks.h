#pragma once

#include "garrysmod/AddonFileSystem.h"
#include "bootil/Bootil.h"
#include "filesystem.h"
#include "steam_api.h"

namespace Addon::Task
{

class AddFloatingAddons : public Addon::Job::Base
{
public:
	~AddFloatingAddons() override = default;
	void Start() override;
	void Cycle() override;
	bool Finished() override;
};

class ClearUnusedGMAs : public Addon::Job::Base
{
public:
	~ClearUnusedGMAs() override = default;
	void Start() override;
	void Cycle() override;
	bool Finished() override;

private:
	bool m_bIsFinished = false;
};

class DownloadAddons : public Addon::Job::Base
{
public:
	~DownloadAddons() override = default;
	void Start() override;
	void Cycle() override;
	bool Finished() override;
};

class DownloadFile : public Addon::Job::Base
{
public:
	~DownloadFile() override = default;
	void Start() override;
	void Cycle() override;
	bool Finished() override;
};

class GetSubscriptions : public Addon::Job::Base
{
public:
	~GetSubscriptions() override = default;
	void Start() override;
	void Cycle() override;
	bool Finished() override;
};

class MountAvailable : public Addon::Job::Base
{
public:
	~MountAvailable() override = default;
	void Start() override;
	void Cycle() override;
	bool Finished() override;
};

class NotifyStart : public Addon::Job::Base
{
public:
	~NotifyStart() override = default;
	void Start() override;
	void Cycle() override;
	bool Finished() override;
};

class NotifyEnd : public Addon::Job::Base
{
public:
	~NotifyEnd() override = default;
	void Start() override;
	void Cycle() override;
	bool Finished() override;
};

class OnSubscribed : public Addon::Job::Base
{
public:
	~OnSubscribed() override = default;
	void Start() override;
	void Cycle() override;
	bool Finished() override;

	OnSubscribed( uint64_t wsid );
private:
	uint64_t m_WSID;
};

class UpdateTotals : public Addon::Job::Base
{
public:
	~UpdateTotals() override = default;
	void Start() override;
	void Cycle() override;
	bool Finished() override;
};

}