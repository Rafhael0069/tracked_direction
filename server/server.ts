import express, { Request, Response, NextFunction, process } from 'express';
const app = express();

const PORT = process.env.PORT || 3000;

app.use(express.json());

// Armazenamento temporário na memória
let data = {
    direc: '',
    but_a: '',
    but_b: '',
    db_le: 0,
    inten: 0,
    tempe: 0
};

// Middleware para tratamento de erros
app.use((err: Error, req: Request, res: Response, next: NextFunction) => {
    console.error('Erro no servidor:', err);
    res.status(500).send('Erro interno do servidor');
});

// Rota para receber os dados do microcontrolador
app.post('/receber', (req: Request, res: Response) => {
    const { direc, but_a, but_b, db_le, inten, tempe } = req.body;

    // Atualiza os dados armazenados
    data = {
        direc: direc || '',
        but_a: but_a || '',
        but_b: but_b || '',
        db_le: db_le || 0,
        inten: inten || 0,
        tempe: tempe || 0
    };

    console.log('Dados atualizados: ', data);
    res.send('Dados recebidos e atualizados com sucesso!');
});

// Nova rota para enviar os dados atuais em JSON
app.get('/dados', (req: Request, res: Response) => {
    res.json(data);
});

// Rota de exemplo para verificar se o servidor está funcionando
app.get('/status', (req: Request, res: Response) => {
    res.send('Servidor está funcionando corretamente!');
});

app.listen(PORT, () => {
    console.log(`Servidor rodando em http://localhost:${PORT}`);
});