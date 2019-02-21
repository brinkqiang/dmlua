-- lua script
module (..., package.seeall)

pb.import("net.proto")

function main()

  local msg = pb.new("net.tb_Person")
  msg.number = "13615632545"
  msg.email = "13615632545@163.com"
  msg.age = 28
  msg.ptype = 2
  msg.desc:add("first")
  msg.desc:add("second")
  msg.desc:add("three")
  print("debug:\n" .. pb.tostring(msg))
  print("bin:\n" .. pb.serializeToString(msg))
end

