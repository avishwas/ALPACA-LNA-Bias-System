.. ALPACA LNA Bias System documentation master file, created by
   sphinx-quickstart on Wed Oct  4 19:26:29 2023.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to ALPACA LNA Bias System's documentation!
==================================================

Login
-----
The username is **root**

The password is **odroid**

User should take precautions such as changing the password and disabling ssh 
at the first opportunity. 

Info
----
The code from this repository utilizes a python virtual environment located in the root home directory.
The python packages may be installed on the system instead of the environment provided they use the requirements.txt
file included. 

Using the environment:

.. code:: bash

   source ~/py3/bin/activate; cd ~/ALPACA-LNA-Bias-System; ipython;

.. code:: python

   import Bias
   Bias.set_I(channel, mA)
   Bias.readV(channel)
   # etc
   # etc

.. toctree::
   :maxdepth: 2
   :caption: API:

   modules

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
