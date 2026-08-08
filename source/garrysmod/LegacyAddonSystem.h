#pragma once

#include "garrysmod/public/ILegacyAddons.h"

class CLegacyAddonSystem : public ILegacyAddons
{
public:
	virtual void Refresh( );
	virtual const std::list<ILegacyAddons::Information> &GetList( ) const;
private:
	std::list<ILegacyAddons::Information> m_pAddons;
};