import express, { Request, Response, NextFunction } from 'express';
const app = express();

const PORT = 3000;

app.use(express.json());

app.use((err: Error, req: Request, res: Response, next: NextFunction) => {
    console.error('Erro no servidor:', err);
    res.status(500).send('Erro interno do servidor');
});

app.post('/receber', (req: Request, res: Response) => {
    console.log('Headers:', req.headers);
    console.log('Body:', req.body);
    const { dado } = req.body;
    console.log('Dado recebido:', dado);
    res.send('Dados recebidos com sucesso!');
});

// Rota de exemplo para verificar se o servidor está funcionando
app.get('/status', (req: Request, res: Response) => {
    res.send('Servidor está funcionando corretamente!');
});

app.listen(PORT, () => {
    console.log('Servidor rodando em http://localhost:3000');
});