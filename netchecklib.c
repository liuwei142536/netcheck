#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
 
 
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/mii.h>
#include <linux/sockios.h>
#include <errno.h>
 
#include <ifaddrs.h>
#include <arpa/inet.h>
 
#include  <linux/ethtool.h>
 
int cshell_netlink_status(char *if_name)
{
	char    buffer[BUFSIZ];
	char    cmd[100];
	FILE    *read_fp;
	int        chars_read;
	int        ret =0;
 
	memset( buffer, 0, BUFSIZ );
	memset( cmd, 0, 100 );
	sprintf(cmd, "ifconfig -a | grep %s",if_name);
	read_fp = popen(cmd, "r");
	if ( read_fp != NULL )
	{
		chars_read = fread(buffer, sizeof(char), BUFSIZ-1, read_fp);
		pclose(read_fp);
 
		if (chars_read > 0)
		{
			ret = 1;
		}
		else
		{
			fprintf(stderr, "DEVICE_NONE\r\n");
			return 0;
		}
	}
 
	if(ret == 1)
	{
		memset( buffer, 0, BUFSIZ );
		memset( cmd, 0, 100 );
		sprintf(cmd, "ifconfig |grep %s",if_name);
		read_fp = popen(cmd, "r");
		if ( read_fp != NULL )
		{
			chars_read = fread(buffer, sizeof(char), BUFSIZ-1, read_fp);
			pclose(read_fp);
 
			if (chars_read > 0)
			{
				ret = 2;
			}
			else
			{
				fprintf(stderr, "DEVICE_DOWN\r\n");
			return 1;
			}
		}
	}
 
	if(ret == 2)
	{
		memset( buffer, 0, BUFSIZ );
		memset( cmd, 0, 100 );
		sprintf(cmd, "ifconfig %s | grep RUNNING |  awk '{print $3}'",if_name);
		read_fp = popen(cmd, "r");
		if ( read_fp != NULL )
		{
		    chars_read = fread(buffer, sizeof(char), BUFSIZ-1, read_fp);
		    pclose(read_fp);
 
		    if (chars_read > 0)
		    {
				fprintf(stderr, "DEVICE_LINKED\r\n");
				return 3;
		    }
		    else
		    {
				fprintf(stderr, "DEVICE_UNPLUGGED\r\n");
				return 2;
		    }
		}
	}
 
	return -1;
}
 
 
int c_netlink_status(const char *if_name )
{
	int fd = -1;
	struct ifreq ifr;
 
	struct ifconf ifc;
	struct ifreq ifrs_buf[100];
	int if_number =0;
	int i;
 
 
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		fprintf(stderr, "%s: socket error [%d] %s\r\n",if_name, errno, strerror(errno));
		close(fd);
		return -1;
	}
 
	ifc.ifc_len = sizeof(ifrs_buf);
	ifc.ifc_buf = (caddr_t)ifrs_buf;
	if (ioctl(fd, SIOCGIFCONF, (char *)&ifc) <0)
	{
		fprintf(stderr, "%s: ioctl SIOCGIFCONF error [%d] %s\r\n",if_name, errno, strerror(errno));
		close(fd);
		return -1;
	}
 
	if_number = ifc.ifc_len / sizeof(struct ifreq);
	for(i=0; i< if_number; i++)
	{
		if(strcmp(if_name,ifrs_buf[i].ifr_name ) == 0)
		{
			break;
		}
	}
 
	if(i >= if_number)
	{
		close(fd);
		fprintf(stderr, "DEVICE_NONE\r\n");
		return 0;
	}
 
	bzero(&ifr, sizeof(ifr));
	strncpy(ifr.ifr_name, if_name, IFNAMSIZ-1);
	ifr.ifr_name[IFNAMSIZ-1] = 0;
	if (ioctl(fd, SIOCGIFFLAGS, (char *)&ifr) <0)
	{
		fprintf(stderr, "%s: ioctl SIOCGIFFLAGS error [%d] %s\r\n",if_name, errno, strerror(errno));
		close(fd);
		return -1;
	}
#if 1
	if(!(ifr.ifr_flags & IFF_UP))
	{
		close(fd);
		fprintf(stderr, "DEVICE_DOWN\r\n");
		return 0;
	}
 
	if(!(ifr.ifr_flags & IFF_RUNNING))
	{
		close(fd);
		fprintf(stderr, "DEVICE_UNPLUGGED\r\n");
		return 0 ;
	}
 
	//fprintf(stderr, "DEVICE_LINKED\r\n");
	return 1;
 
#else
{
	struct ethtool_value edata;
	if(!(ifr.ifr_flags & IFF_UP) || !(ifr.ifr_flags & IFF_RUNNING))
	{
		close(fd);
		fprintf(stderr, "%s: DOWN\r\n",if_name);
		return 1;
	}
	edata.cmd = ETHTOOL_GLINK;
	edata.data = 0;
	ifr.ifr_data = (char *) &edata;
	if(ioctl( fd, SIOCETHTOOL, &ifr ) < 0)
	{
		fprintf(stderr, "%s: ioctl SIOCETHTOOL error [%d] %s\r\n",if_name, errno, strerror(errno));
		close(fd);
		return -1;
	}
 
	if(edata.data == 0)
	{
		fprintf(stderr, "DEVICE_UNPLUGGED\r\n");
		return 2;
	}
	else
	{
		fprintf(stderr, "DEVICE_LINKED\r\n");
		return 3;
	}
}
#endif
}
 
int netLinkCheck(char *netcardName)
{
	int i=0;
 
	//i = cshell_netlink_status(netcardName);
 
	//printf( "cshell_netlink_status if_status = %d\n", i );
 
	i = c_netlink_status(netcardName);
	//printf( "c_netlink_status if_status = %d\n", i );
 
	return i;
}
