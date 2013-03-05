import ebftpd

if __name__ == "__main__":
  print "ebftpd rocks!"
  
  user = ebftpd.User.load("w00t")
  if user:
    print user.name
  else:
    print "dosen't exist"

  user.rename("w00t")

  print type(user.ip_masks)
  user.del_ip_mask("gjdfkjgkdfjgfd")
  user.del_ip_mask("!!!!")
  for ip in user.ip_masks:
    print ip, type(ip)

  for gid in user.secondary_gids():
    print gid

  print user.created
  
  print user.last_login
  print user.default_credits
  print user.get_section_credits("MP3")
#  user.incr_section_credits("MP3", 1000000)
  print user.get_section_credits("MP3")

  print user.tagline
  user.tagline = "omfg!"
  print user.tagline
  assert user.verify_password("ebftpd")
  assert not user.verify_password("gfgdfgdf")
  print user.secondary_gids()
  print user.primary_group
  user.add_gids([1,2,3])
  print user.secondary_gids()
  print ebftpd.User.get_uids()
  print ebftpd.User.total_users()

  for user in ebftpd.User.get_users():
    print user.name

  user.incr_default_credits(100000)


