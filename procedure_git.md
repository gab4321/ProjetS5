# Projet S5
## Comment utiliser Git
Petites instructions pour ceux qui ne sont pas familiers avec les systèmes de versionnage. Vous pouvez me poser des questions si vous avez des problèmes.
### Installer SourceTree
Je conseille d'installer [SourceTree](https://www.sourcetreeapp.com/). Git peut être utilisé en command line, mais c'est toujours pratique d'avoir un outil visuel.

### Cloner le repository
Cloner le repository à l'endroit de votre choix. ([Documentation](https://confluence.atlassian.com/sourcetreekb/clone-a-repository-into-sourcetree-780870050.html) pour SourceTree) en utilisant l'URL HTTPS ou SSH sur cette page. Je conseille HTTPS pour ceux qui ne savent pas comment générer des clés SSH. Vous allez seulement devoir entrer votre mot de passe et username plus souvent. Si certains veulent des informations pour le SSH, je peux vous expliquer.

### Procédure pour travailler
1. Faire **pull** avant de commencer à travailler
2. Modifier et/ou ajouter des fichiers
3. Aller dans **Working Copy** et vérifier que les modifications faites sont voulues et qu'il n'y a pas d'oublies
4. Cochez les fichiers prêts à commiter pour les **stager**
5. Toujours faire **pull** avant de commiter pour éviter les problèmes
6. Entrer un message significatif et faire **commit**. Seulement les fichiers stagés seront commités.
7. Faire **push**. _IMPORTANT_ : C'est à ce moment seulement que les changements **_commités_** sont envoyés sur le repository. Autrement, les commits faits ne sont que locals. Les fichiers non commités ne sont pas pushés sur le repository.

_Note_ : Vous pouvez faire plusieurs *commits* (qui sont gardé seulement en local) et faire un **push** par la suite. Toutefois, c'est recommandé de pusher à chaque fois que vous êtes confiants de ce que vous avez fait.
