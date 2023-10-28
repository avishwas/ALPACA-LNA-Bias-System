.. ALPACA LNA Bias System documentation master file, created by
   sphinx-quickstart on Wed Oct  4 19:26:29 2023.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

ASU ALPACA Bias Documentation
=============================

Login
-----
The username is **root**

The password is **odroid**

User should take precautions such as changing the password and disabling ssh 
at the first opportunity. 

Intro
-----
The code from this repository was developed using a python virtual environment located in the root home directory.
The packages have since been installed outside of an environment. As a result, the user should not have to use the venv. 
It is however, still recommended.

On import/run, the ALPACABias.py module automatically tries to initialize all of the boards in the system.
During this process all of the bias outputs are set to their minimum. 

For details on usage or documentation, please see the API.

**Please note that this module requires root access.**


Getting Updates
---------------
.. code:: bash

   cd ~/ALPACA-LNA-Bias-System; git pull;


Using ipython
-------------
.. code:: bash

   source ~/py3/bin/activate; cd ~/ALPACA-LNA-Bias-System; ipython;

.. code:: python

   run ALPACABias.py
   set_iLNA(23, 15.0)
   set_iLNA(24, 15.0)
   get_iLNA(1)
   get_vLNA(144)
   get_iLNA(144)
   # etc, etc, etc


.. toctree::
   :maxdepth: 2
   :caption: API:

   modules

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
