# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure(2) do |config|
  config.vm.box = "ubuntu/trusty64"

  config.vm.provider "parallels" do |vb, override|
    override.vm.box = "parallels/ubuntu-14.04"

    vb.memory = 1024
    vb.cpus = 2
  end

  config.vm.box_check_update = false
  config.ssh.insert_key = false
  config.ssh.private_key_path = File.expand_path('~/.vagrant.d/insecure_private_key')
  config.ssh.forward_agent= true

  boxes = [
    { :name => "webserver",      :ip => "192.168.3.10" },
  ]

  boxes.each do |opts|
    config.vm.define opts[:name] do |config|
        config.vm.hostname = opts[:name]
        config.vm.network :private_network, ip: opts[:ip]

        config.vm.network "forwarded_port", guest: 7009, host: 7009
        # config.vm.synced_folder "workspace/g2u-backend", "/data/huanlian-home/g2u-backend", owner: "www-data", group: "www-data"
        
    end
  end
end
