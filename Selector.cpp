#include <Selector.hpp>
#include <Device.hpp>
#include <algorithm>

Selector::Selector()
	: fd_max(-1)
{
	FD_ZERO(&rfds_cache);
}

Selector::~Selector()
{}

void Selector::set_fd_max()
{
	fd_max = -1;
	for (Devices::const_iterator i = devices.begin(); i != devices.end(); ++i)
		if ((*i)->fd > fd_max)
			fd_max = (*i)->fd;
}

void Selector::add(Device * device)
{
	if (!device) return;
	if (std::find(devices.begin(), devices.end(), device) != devices.end()) return;
	devices.push_back(device);
	FD_SET(device->fd, &rfds_cache);
	if (device->fd> fd_max) fd_max = device->fd;
}

void Selector::remove(Device * device)
{
	if (!device) return;
	Devices::iterator i = std::find(devices.begin(), devices.end(), device);
	if (i == devices.end()) return;
	devices.erase(i);
	FD_CLR(device->fd, &rfds_cache);
	set_fd_max();
}

int Selector::select(Device ** device)
{
	// TODO: timeout
	// TODO: signals (pselect)

	if (!device) return -1;

	fd_set rfds = rfds_cache;

	int rc = ::pselect(fd_max, &rfds, NULL, NULL, NULL, NULL);
	if (rc < 0) return -1;

	for (Devices::iterator i = devices.begin(); i != devices.end(); ++i) {
		if (FD_ISSET((*i)->fd, &rfds)) {
			*device = *i;
			return 0;
		}
	}
	return -1;
}

int Selector::select(Device & device)
{
	// TODO: timeout
	// TODO: signals (pselect)

	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(device.fd, &rfds);
	int rc = ::pselect(device.fd+1, &rfds, NULL, NULL, NULL, NULL);
	if (rc < 0) return -1;
	return 0;
}

